/**
 * implied_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "implied_map.h"

#include "map_core.h"
#include "representation.h"
#include "symmetrized_matrix_system.h"

#include "scenarios/context.h"

#include "symbolic/symbol_table.h"
#include "utilities/dynamic_bitset.h"

#include <limits>
#include <sstream>

namespace Moment::Symmetrized {

    namespace {
        constexpr bool is_close(const double x, const double y, const double eps_mult = 1.0) {
            return (std::abs(x - y) < std::numeric_limits<double>::epsilon() * std::max(std::abs(x), std::abs(y)));
        }

        std::pair<std::vector<symbol_name_t>, DynamicBitset<size_t>>
        unzip_indices(const SymbolTable& origin_symbols, size_t matrix_size) {
            auto output = std::make_pair(std::vector<symbol_name_t>(),  DynamicBitset<size_t>{matrix_size, false});
            output.first.reserve(matrix_size);

            for (size_t i = 0; i < matrix_size; ++i) {
                auto [symbol_id, conjugate] = origin_symbols.OSGIndex(i);
                output.first.emplace_back(symbol_id);
                if (conjugate) {
                    output.second.set(i);
                }
            }
            return output;
        }
    }


    ImpliedMap::ImpliedMap(SymmetrizedMatrixSystem& sms, std::unique_ptr<MapCore> core_in,
                           std::unique_ptr<SolvedMapCore> solution_in)
        : origin_symbols{sms.base_system().Symbols()}, target_symbols{sms.Symbols()},
            core{std::move(core_in)}, core_solution{std::move(solution_in)} {
        if (!this->core) {
            throw errors::bad_map{"Map cannot be constructed without a MapCore."};
        }
        if (!this->core_solution) {
            throw errors::bad_map{"Map cannot be constructed without a SolvedMapCore."};
        }


        auto [osg_to_sym, conjugates] = unzip_indices(sms.base_system().Symbols(), this->core->initial_size);
        this->construct_map(osg_to_sym);
    }

    ImpliedMap::ImpliedMap(SymmetrizedMatrixSystem& sms,
                           MapCoreProcessor&& processor, const Eigen::MatrixXd& src)
        : origin_symbols{sms.base_system().Symbols()}, target_symbols{sms.Symbols()} {

        auto [osg_to_sym, conjugates] = unzip_indices(sms.base_system().Symbols(), src.cols());

        this->core = std::make_unique<MapCore>(std::move(conjugates), src);
        this->core_solution = core->accept(std::move(processor));
        this->construct_map(osg_to_sym);
    }

    ImpliedMap::ImpliedMap(SymmetrizedMatrixSystem& sms,
                           MapCoreProcessor&& processor, const Eigen::SparseMatrix<double>& src)
        : origin_symbols{sms.base_system().Symbols()}, target_symbols{sms.Symbols()} {

        auto [osg_to_sym, conjugates] = unzip_indices(sms.base_system().Symbols(), src.cols());

        this->core = std::make_unique<MapCore>(std::move(conjugates), src);
        this->core_solution = core->accept(std::move(processor));
        this->construct_map(osg_to_sym);
    }


    void ImpliedMap::construct_map(const std::vector<symbol_name_t>& osg_to_symbols) {
        assert(&origin_symbols != &target_symbols);
        assert(this->core);
        assert(this->core_solution);

        // First, build fixed constants
        this->map.assign(origin_symbols.size(), SymbolCombo::Zero()); // 0 -> 0 always
        this->map[0] = SymbolCombo::Zero();      // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

        // Create scalars remap:
        for (auto [row_id, scalar] : this->core->constants) {
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(row_id);
            assert(!conjugated);
            this->map[symbol_id] = SymbolCombo::Scalar(scalar);
        }

        const auto& raw_map = this->core_solution->map;
        const auto& raw_inv_map = this->core_solution->inv_map;

        // Create non-trivial map:
        Eigen::Index core_col_id = 0;
        for (auto non_trivial_idx : this->core->nontrivial_cols) {

            const symbol_name_t source_symbol = osg_to_symbols[core_col_id];

            SymbolCombo::storage_t from_x_to_y;

            // Constant offset, if any:
            if (this->core->core_offset[core_col_id] != 0.0) {
                from_x_to_y.emplace_back(static_cast<symbol_name_t>(1), this->core->core_offset[core_col_id]);
            }
            // Non-trivial parts
            for (Eigen::Index map_col_id = 0, map_col_max = raw_map.cols();
                map_col_id < map_col_max; ++map_col_id) {
                const double value = raw_map(core_col_id, map_col_id);
                if (abs(value) != 0.0) {
                    from_x_to_y.emplace_back(osg_to_symbols[map_col_id], value);
                }
            }
            // Create mapping
            this->map[source_symbol] = SymbolCombo{std::move(from_x_to_y)};
            ++core_col_id;
        }

        // Create reverse map:
        this->inverse_map.assign(2 + this->core_solution->output_symbols, SymbolCombo::Zero());
        this->inverse_map[0] = SymbolCombo::Zero();      // 0 -> 0 always.
        this->inverse_map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.
        // TODO: Create reverse map

    }


    ImpliedMap::~ImpliedMap() noexcept = default;


    const SymbolCombo& ImpliedMap::operator()(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in implied map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->map[symbol_id];
    }

    SymbolCombo ImpliedMap::operator()(const SymbolExpression &symbol) const {
        // Get raw combo, or throw range error
        SymbolCombo output = (*this)(symbol.id);

        // Apply transformations (using target symbol table)
        output *= symbol.factor;
        if (symbol.conjugated) {
            output.conjugate_in_place(this->target_symbols);
        }

        return output;
    }

    const SymbolCombo& ImpliedMap::inverse(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->inverse_map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in inverse map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->inverse_map[symbol_id];
    }

    SymbolCombo ImpliedMap::inverse(const SymbolExpression& symbol) const {
        // Get raw combo, or throw range error
        SymbolCombo output = this->inverse(symbol.id);

        // Apply transformations (using target symbol table)
        output *= symbol.factor;
        if (symbol.conjugated) {
            output.conjugate_in_place(this->target_symbols);
        }

        return output;
    }
}