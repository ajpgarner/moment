/**
 * implied_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "implied_map.h"

#include "symbolic/symbol_table.h"
#include "representation.h"

#include <limits>
#include <sstream>

namespace Moment::Symmetrized {

    namespace {
        constexpr bool is_close(const double x, const double y, const double eps_mult = 1.0) {
            return (abs(x - y) < std::numeric_limits<double>::epsilon() * std::max(abs(x), abs(y)));
        }

    }

    ImpliedMap::ImpliedMap(const SymbolTable& origin_symbols, SymbolTable& target_symbols,
                           const Representation& rep)
       : origin_symbols{origin_symbols}, target_symbols{target_symbols}, max_length{rep.word_length} {
        assert(this->origin_symbols.size() >= 2);
        assert(&origin_symbols != &target_symbols);

        // TODO: Check if we can generate a map using this representation for this symbol table.


        // Get group average
        const auto& raw_remap = rep.sum_of(); // off by factor of rep.size() - to avoid division.

        // First, build constants
        this->map[0] = SymbolCombo::Zero(); // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

        // First, check column 0 just maps 1 -> 1
        if ((raw_remap.col(0).nonZeros() != 1) || !is_close(raw_remap.coeff(0,0), static_cast<double>(rep.size()))) {
            throw errors::bad_map{"Column 0 of transformation should /only/ contain the identity."};
        }

        // Now, check remaining columns for constants
        std::vector<bool> trivial(raw_remap.cols(), false);
        trivial[0] = true;
        for (int col_index = 1; col_index < raw_remap.cols(); ++col_index) {
            repmat_t::Index nnz = raw_remap.col(col_index).nonZeros();
            if (0 == nnz) {

            }

            //raw_remap.col()
        }


        throw std::runtime_error{"ImpliedMap::ImpliedMap not yet implemented."};

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
}