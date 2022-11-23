/**
 * factor_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "factor_table.h"

#include "inflation_context.h"

#include "operators/matrix/symbol_table.h"

namespace NPATK {
    FactorTable::FactorTable(const InflationContext& context, SymbolTable& symbols_in)
        : context{context}, symbols{symbols_in} {
        this->check_for_new_factors();
    }

    size_t FactorTable::check_for_new_factors() {
        const auto next_id = static_cast<symbol_name_t>(this->entries.size());
        const auto up_to_id = static_cast<symbol_name_t>(this->symbols.size());

        // Early exit, if no new symbols
        if (next_id == up_to_id) {
            return 0;
        }

        // Check for factors of new symbols
        for (symbol_name_t symbol_index = next_id; symbol_index < up_to_id; ++symbol_index) {
            const auto& symbol = this->symbols[symbol_index];
            this->entries.emplace_back();
            auto& entry = this->entries.back();
            entry.id = symbol.Id();
            entry.sequences = this->context.factorize(symbol.sequence());

            // Look up symbols
            entry.symbols.reserve(entry.sequences.size());
            for (const auto& factor_seq : entry.sequences) {
                auto where = this->symbols.where(factor_seq);
                if (where != nullptr) {
                    assert(where->is_hermitian());
                    entry.symbols.emplace_back(where->Id());
                } else {
                    UniqueSequence us{factor_seq, context.hash(factor_seq)};
                    auto new_entry = this->symbols.merge_in(std::move(us));
                    entry.symbols.emplace_back(new_entry);
                }
            }
        }

        // Newly added symbols automatically will not factorize
        const auto extra_symbols = static_cast<symbol_name_t>(this->symbols.size());
        for (symbol_name_t symbol_index = up_to_id; symbol_index < extra_symbols; ++symbol_index) {
            const auto& symbol = this->symbols[symbol_index];
            this->entries.emplace_back();
            auto& entry = this->entries.back();
            entry.id = symbol.Id();
            entry.sequences = std::vector<OperatorSequence>{symbol.sequence()};
            entry.symbols = std::vector<symbol_name_t>{entry.id};
        }

        return extra_symbols - next_id;
    }
}