/**
 * defining_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "symbol_table_map.h"

#include "map_core.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "scenarios/context.h"

#include "symbolic/symbol_table.h"
#include "utilities/dynamic_bitset.h"
#include "utilities/float_utils.h"

#include <limits>
#include <ranges>
#include <sstream>

namespace Moment::Derived {

    namespace {

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


    SymbolTableMap::SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                                   std::unique_ptr<MapCore> core_in,
                                   std::unique_ptr<SolvedMapCore> solution_in)
        : origin_symbols{origin}, target_symbols{target},
            core{std::move(core_in)}, core_solution{std::move(solution_in)} {
        if (!this->core) {
            throw errors::bad_map{"Map cannot be constructed without a MapCore."};
        }
        if (!this->core_solution) {
            throw errors::bad_map{"Map cannot be constructed without a SolvedMapCore."};
        }

        auto [osg_to_sym, conjugates] = unzip_indices(this->origin_symbols, this->core->initial_size);
        this->construct_map(osg_to_sym, conjugates);
        this->populate_target_symbols();
    }

    SymbolTableMap::SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                                   const MapCoreProcessor& processor, const Eigen::MatrixXd& src)
        : origin_symbols{origin}, target_symbols{target} {

        auto [osg_to_sym, conjugates] = unzip_indices(this->origin_symbols, src.cols());

        this->core = std::make_unique<DenseMapCore>(conjugates, src);
        this->core_solution = core->accept(processor);
        this->construct_map(osg_to_sym, conjugates);
        this->populate_target_symbols();
    }

    SymbolTableMap::SymbolTableMap(const SymbolTable& origin, SymbolTable& target,
                                   const MapCoreProcessor& processor, const Eigen::SparseMatrix<double>& src)
        : origin_symbols{origin}, target_symbols{target} {

        auto [osg_to_sym, conjugates] = unzip_indices(this->origin_symbols, src.cols());

        this->core = std::make_unique<DenseMapCore>(conjugates, src);
        this->core_solution = core->accept(processor);
        this->construct_map(osg_to_sym, conjugates);
        this->populate_target_symbols();
    }


    void SymbolTableMap::construct_map(const std::vector<symbol_name_t>& osg_to_symbols,
                                       const DynamicBitset<size_t>& osg_conjugate) {
        assert(this->core);
        assert(this->core_solution);

        // Check core and solution match.
        this->core->check_solution(*this->core_solution);

        // First, build fixed constants.
        this->map.assign(origin_symbols.size(), SymbolCombo::Zero()); // 0 -> 0 always
        this->map[0] = SymbolCombo::Zero();      // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

        // Create scalars remap:
        for (auto [row_id, scalar] : this->core->constants) {
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(row_id);
            assert(!conjugated);
            this->map[symbol_id] = SymbolCombo::Scalar(scalar);
        }

        const auto& raw_map = this->core_solution->dense_map;
        const auto& raw_inv_map = this->core_solution->dense_inv_map;

        // Create non-trivial map:
        Eigen::Index core_col_id = 0;
        for (auto non_trivial_idx : this->core->nontrivial_cols) {

            const symbol_name_t source_symbol = osg_to_symbols[non_trivial_idx];

            SymbolCombo::storage_t from_x_to_y;

            // Constant offset, if any:
            const double offset = this->core->core_offset[core_col_id];
            if (!approximately_zero(offset)) {
                from_x_to_y.emplace_back(static_cast<symbol_name_t>(1), offset);
            }

            // Non-trivial parts
            for (Eigen::Index map_col_id = 0, map_col_max = raw_map.cols();
                map_col_id < map_col_max; ++map_col_id) {
                const auto as_symbol = static_cast<symbol_name_t>(map_col_id + 2);
                const double value = raw_map(core_col_id, map_col_id);
                if (!approximately_zero(value)) {
                    from_x_to_y.emplace_back(as_symbol, value);
                }
            }

            // Create mapping
            this->map[source_symbol] = SymbolCombo{std::move(from_x_to_y)};
            ++core_col_id;
        }

        // Check if map is monomial
        this->_is_monomial_map = std::all_of(this->map.cbegin(), this->map.cend(),
                                            [](const auto& combo) { return combo.is_monomial(); });

        // Create reverse map:
        this->inverse_map.reserve(2 + this->core_solution->output_symbols);
        this->inverse_map.emplace_back(SymbolCombo::Zero());      // 0 -> 0 always.
        this->inverse_map.emplace_back(SymbolCombo::Scalar(1.0)); // 1 -> 1 always.

        for (Eigen::Index im_row_id = 0; im_row_id < this->core_solution->output_symbols; ++im_row_id) {
            SymbolCombo::storage_t from_y_to_x;
            assert(this->core->nontrivial_rows.count() == this->core_solution->dense_inv_map.cols());

            Eigen::Index im_col_id = 0;
            for (auto non_trivial_idx : this->core->nontrivial_rows) {
                const double value = raw_inv_map(im_row_id, im_col_id);
                if (abs(value) != 0.0) {
                    // Map: Core index -> osg index -> symbol table ID
                    from_y_to_x.emplace_back(osg_to_symbols[non_trivial_idx], value, osg_conjugate[non_trivial_idx]);
                }
                ++ im_col_id;
            }
            this->inverse_map.emplace_back(std::move(from_y_to_x));
        }
        assert(this->inverse_map.size() == this->core_solution->output_symbols+2);
    }

    void SymbolTableMap::populate_target_symbols() {
        // Function occurs in context of construction of a derived matrix system.
        // target_symbols should not yet be publicly visible elsewhere, so should be automatically thread safe.
        // origin_symbols should be read-locked as part of the origin matrix system lock.

        if (2 != this->target_symbols.size()) {
            throw errors::bad_map{"Target SymbolTable should be empty (except for zero and identity)."};
        }

        if (this->inverse_map.size() < 2) {
            throw errors::bad_map{"Inverse map must define zero and identity."};
        }

        for (const auto& polynomial : this->inverse_map | std::views::drop(2)) {
            const bool is_hermitian = polynomial.is_hermitian(this->origin_symbols);
            this->target_symbols.create(true, !is_hermitian, polynomial.as_string());
        }

        assert(this->target_symbols.size() == this->inverse_map.size());
    }



    SymbolTableMap::~SymbolTableMap() noexcept = default;


    const SymbolCombo& SymbolTableMap::operator()(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in implied map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->map[symbol_id];
    }

    SymbolCombo SymbolTableMap::operator()(const SymbolExpression &symbol) const {
        // Get raw combo, or throw range error
        SymbolCombo output = (*this)(symbol.id);

        // Apply transformations (using target symbol table)
        output *= symbol.factor;
        if (symbol.conjugated) {
            output.conjugate_in_place(this->target_symbols);
        }

        return output;
    }

    SymbolCombo SymbolTableMap::operator()(const SymbolCombo &symbol) const {
        SymbolCombo::storage_t joint_storage{};
        for (const auto& expr : symbol) {
            const auto tx_symbol = (*this)(expr);
            joint_storage.insert(joint_storage.end(), tx_symbol.begin(), tx_symbol.end());
        }

        return SymbolCombo{std::move(joint_storage)};
    }


    std::unique_ptr<SquareMatrix<SymbolCombo>>
    SymbolTableMap::operator()(const SquareMatrix<SymbolExpression>& input_matrix) const {
        std::vector<SymbolCombo> output_data;
        output_data.reserve(input_matrix.dimension * input_matrix.dimension);
        for (const auto& expr : input_matrix) {
            output_data.emplace_back((*this)(expr));
        }
        return std::make_unique<SquareMatrix<SymbolCombo>>(input_matrix.dimension, std::move(output_data));
    }


    [[nodiscard]] std::unique_ptr<SquareMatrix<SymbolCombo>>
    SymbolTableMap::operator()(const SquareMatrix<SymbolCombo>& input_matrix) const {
        std::vector<SymbolCombo> output_data;
        output_data.reserve(input_matrix.dimension * input_matrix.dimension);
        for (const auto& combo : input_matrix) {
            output_data.emplace_back((*this)(combo));
        }
        return std::make_unique<SquareMatrix<SymbolCombo>>(input_matrix.dimension, std::move(output_data));
    }

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    SymbolTableMap::monomial(const SquareMatrix<SymbolExpression>& input_matrix) const {
        if (!this->_is_monomial_map) {
            throw errors::bad_map{"Cannot create monomial matrix from action of non-monomial map."};
        }
        std::vector<SymbolExpression> output_data;
        output_data.reserve(input_matrix.dimension * input_matrix.dimension);
        for (const auto& expr : input_matrix) {
            output_data.emplace_back(SymbolExpression{(*this)(expr)});
        }
        return std::make_unique<SquareMatrix<SymbolExpression>>(input_matrix.dimension, std::move(output_data));
    }

    const SymbolCombo& SymbolTableMap::inverse(symbol_name_t symbol_id) const {
        // Bounds check
        if ((symbol_id < 0) || (symbol_id >= this->inverse_map.size())) {
            std::stringstream ss;
            ss << "Symbol " << symbol_id << " not defined in inverse map.";
            throw errors::bad_map{ss.str()};
        }

        // Remap
        return this->inverse_map[symbol_id];
    }

    SymbolCombo SymbolTableMap::inverse(const SymbolExpression& symbol) const {
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