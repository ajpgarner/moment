/**
 * derived_context.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "derived_context.h"

#include "symbolic/symbol_table.h"

#include <iostream>

namespace Moment::Derived {

    DerivedContext::DerivedContext(const Context& source_context)
        : Context(0), base_context{source_context} {

    }

    void DerivedContext::format_symbol(std::ostream &os, const SymbolTable &table,
                                       const symbol_name_t symbol_id) const {
        if (symbol_id < 0 || symbol_id >= table.size()) {
            os << "[UNK: #" << symbol_id << "]";
            return;
        }

        // Get derived strings
        assert(symbol_id < this->derived_symbol_strs.size());
        os << this->derived_symbol_strs[symbol_id];
    }

}