/**
 * implied_map.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "symbolic/symbol_combo.h"

#include <stdexcept>
#include <string>
#include <vector>

#include <cassert>

namespace Moment {

    class SymbolTable;

    namespace errors {
        class bad_map : public std::range_error {
        public:
            bad_map(const std::string& what) : std::range_error{what} { }
        };
    }

    namespace Symmetrized {
        class Representation;

        class ImpliedMap {
        private:
            const SymbolTable& origin_symbols;
            const SymbolTable& target_symbols;

            std::vector<SymbolCombo> map;

        public:
            ImpliedMap(const SymbolTable& origin_symbols, const SymbolTable& target_symbols, const Representation& rep);

            /**
             * Get symbol/symbol combo in target, associated with symbol in source.
             * @param symbol_id Source symbol ID
             * @return Reference to symbol combo.
             * @throws error::bad_map If symbol id is out of range.
             */
            const SymbolCombo& operator()(symbol_name_t symbol_id) const;

            /**
              * Get symbol/symbol combo in target, associated with symbol expression in source.
              * Also takes into account prefactors/complex conjugation etc.
              * @param symbol_id Source symbol
              * @return New symbol combo, transforming the supplied expression.
              * @throws error::bad_map If symbol is out of range.
              */
            SymbolCombo operator()(const SymbolExpression& symbol) const;


        };
    };

}