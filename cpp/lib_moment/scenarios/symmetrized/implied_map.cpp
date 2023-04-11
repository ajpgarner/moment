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

    ImpliedMap::ImpliedMap(const SymbolTable& origin_symbols, const SymbolTable& target_symbols,
                           const Representation& rep)
       : origin_symbols{origin_symbols}, target_symbols{target_symbols} {

        const auto& average = rep.sum_of(); // off by factor of rep.size() - to avoid division.

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