/**
 * implied_map.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "implied_map.h"

#include "symbolic/symbol_table.h"
#include "representation.h"

#include <sstream>

namespace Moment::Symmetrized {

    ImpliedMap::ImpliedMap(const SymbolTable& origin_symbols, SymbolTable& target_symbols,
                           const Representation& rep)
       : origin_symbols{origin_symbols}, target_symbols{target_symbols}, map(origin_symbols.size()) {
        assert(this->origin_symbols.size() > 2);
        assert(&origin_symbols != &target_symbols);

        // Get group average
        const auto& average = rep.sum_of(); // off by factor of rep.size() - to avoid division.

        // First, build constants
        this->map[0] = SymbolCombo::Zero(); // 0 -> 0 always.
        this->map[1] = SymbolCombo::Scalar(1.0); // 1 -> 1 always.

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