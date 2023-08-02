/**
 * derived_context.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "derived_context.h"

#include "symbol_table_map.h"

#include "symbolic/symbol_table.h"
#include <iostream>

namespace Moment::Derived {

    DerivedContext::DerivedContext(const Context& source_context)
        : Context(0), base_context{source_context} {

    }

    void DerivedContext::format_sequence_from_symbol_id(ContextualOS& os,
                                                        const symbol_name_t symbol_id,
                                                        bool conjugated) const {
        assert(this->map_ptr);
        // If out of range, do nothing
        if ((symbol_id < 0) || (symbol_id >= this->map_ptr->inv_size())) {
            Context::format_sequence_from_symbol_id(os, symbol_id, conjugated);
            return;
        }

        // Make context from old MS
        assert(this->sfc.has_value());
        ContextualOS baseCOS{os.os, this->sfc.value()};
        baseCOS.format_info.locality_formatter = os.format_info.locality_formatter;

        // Use polynomials from STM to infer inverse
        if (conjugated) {
            auto poly = this->map_ptr->inverse(Monomial{symbol_id, 1.0, true});
            baseCOS << poly;
        } else {
            const auto& poly = this->map_ptr->inverse(symbol_id);
            baseCOS << poly;
        }
    }

    void DerivedContext::set_symbol_table_map(const class SymbolTableMap* new_map_ptr) noexcept {
        assert(this->map_ptr == nullptr);
        assert(!this->sfc.has_value());

        this->map_ptr = new_map_ptr;
        this->sfc.emplace(this->base_context, this->map_ptr->Origin());
        this->sfc->format_info.show_braces = true;
        this->sfc->format_info.display_symbolic_as = StringFormatContext::DisplayAs::Operators;
    }

}