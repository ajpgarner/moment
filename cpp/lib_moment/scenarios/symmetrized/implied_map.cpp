/**
 * implied_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "implied_map.h"

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
            return (abs(x - y) < std::numeric_limits<double>::epsilon() * std::max(abs(x), abs(y)));
        }

    }


    ImpliedMap::NontrivialMapCore::NontrivialMapCore(const SymbolTable& origin_symbols,
                                                     const Moment::Symmetrized::repmat_t& raw_remap)
        : nontrivial_rows{static_cast<size_t>(raw_remap.cols()), false},
          nontrivial_cols{static_cast<size_t>(raw_remap.cols()), true} {

        this->nontrivial_cols[0] = false;

        for (int col_index = 1; col_index < raw_remap.cols(); ++col_index) {
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(col_index);

            // Identify complex conjugate rows
            if (conjugated) {
                this->nontrivial_cols[col_index] = false;
                this->conjugates.emplace(col_index);
                continue;
            }

            // Identify rows with no values, or only a constant value:
            repmat_t::Index nnz = raw_remap.col(col_index).nonZeros();
            if (0 == nnz) {
                this->constants.emplace(std::make_pair(col_index, 0));
                this->nontrivial_cols[col_index] = false;
                continue;
            } else if (1 == nnz) {
                const double offset_term = raw_remap.coeff(0, col_index);
                if (offset_term > 0) {
                    this->constants.emplace(std::make_pair(col_index, offset_term));
                    this->nontrivial_cols[col_index] = false;
                    continue;
                }
            }

            // Otherwise, column is nontrivial - identify rows that are nontrivial
            for (auto row_iter = repmat_t::InnerIterator{raw_remap, col_index}; row_iter; ++row_iter) {
                this->nontrivial_rows[row_iter.row()] = true;
            }
        }

        auto remapped_cols = static_cast<Eigen::Index>(this->nontrivial_cols.count());
        auto remapped_rows = static_cast<Eigen::Index>(this->nontrivial_rows.count());

        // Copy dense matrix
        this->core.resize(remapped_rows, remapped_cols);
        auto *write_iter = this->core.data();
        for (const auto old_col_idx : this->nontrivial_cols) {
            for (const auto old_row_idx : this->nontrivial_rows) {
                *(write_iter) = raw_remap.coeff(static_cast<Eigen::Index>(old_row_idx),
                                                static_cast<Eigen::Index>(old_col_idx));
                ++write_iter;
            }
        }

    }

    ImpliedMap::ImpliedMap(SymmetrizedMatrixSystem& sms,
                           const Representation& rep)

       : origin_symbols{sms.base_system().Symbols()}, target_symbols{sms.Symbols()}, max_length{rep.word_length} {
        assert(&origin_symbols != &target_symbols);

        // Check length of origin symbols is sufficiently long:
        const size_t osg_length = origin_symbols.OSGIndex.max_length();
        if (rep.word_length > osg_length) {
            std::stringstream errSS;
            errSS << "Symbol only guarantees dictionary up to length " << osg_length
                  << " but representation of length " << rep.word_length << " was supplied.";
            throw errors::bad_map{errSS.str()};
        }


        // TODO: Pass OSG and some-way of mapping OSG indices to symbol table indices.

        // Get number of elements


        // First, build constants
        this->map.assign(origin_symbols.size(), SymbolCombo::Zero()); // 0 -> 0 always
        this->map[0] = SymbolCombo::Zero();      // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

        // Get group average and check first column maps 1 -> 1
        const auto& raw_remap = rep.sum_of(); // = average * rep.size() - to avoid division.
        if ((raw_remap.col(0).nonZeros() != 1) || !is_close(raw_remap.coeff(0,0), static_cast<double>(rep.size()))) {
            throw errors::bad_map{"Column 0 of transformation should /only/ contain the identity."};
        }

        // Extract non-trivial subcomponent
        this->nontrivial_core = std::make_unique<NontrivialMapCore>(this->origin_symbols, raw_remap);

        // Create scalar remap:
        for (auto [row_id, scalar] : this->nontrivial_core->constants) {
            auto [symbol_id, conjugated] = origin_symbols.OSGIndex(row_id);
            assert(!conjugated);
            this->map[symbol_id] = SymbolCombo::Scalar(scalar / static_cast<double>(rep.size()));
        }

        this->process_nontrivial_core();

    }

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

    void ImpliedMap::process_nontrivial_core() {
        assert(this->nontrivial_core);


    }

}