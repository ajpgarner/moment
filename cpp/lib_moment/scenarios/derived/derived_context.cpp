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

    void DerivedContext::format_sequence_from_symbol_id(ContextualOS& os,
                                                        const symbol_name_t symbol_id,
                                                        bool conjugated) const {
        assert(dynamic_cast<const DerivedContext*>(&os.context) == this);

        // Get derived strings if in bounds:
        if ((symbol_id >= 0) && (symbol_id < this->derived_symbol_strs.size())) {
            if (conjugated) {
                os << this->derived_symbol_strs[symbol_id]; // TODO: Conjugated strings.
            } else {
                os << this->derived_symbol_strs[symbol_id];
            }
        } else {
            Context::format_sequence_from_symbol_id(os, symbol_id, conjugated);
        }
    }

}