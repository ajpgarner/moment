/**
 * factor_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "factor_table.h"

#include "inflation_context.h"

#include "symbolic/symbol_table.h"

#include <sstream>

namespace Moment {

    std::string FactorTable::FactorEntry::sequence_string() const {

        std::stringstream ss;

        if (this->canonical.sequences.size() == 1) {
            if (this->canonical.sequences[0].empty()) {
                if (this->canonical.sequences[0].zero()) {
                    return "0";
                } else {
                    return "1";
                }
            }
        }

        for (const auto& seq : this->canonical.sequences) {
            ss << "<" << seq << ">";
        }
        return ss.str();
    }


    FactorTable::FactorTable(const InflationContext& context, SymbolTable& symbols_in)
        : context{context}, symbols{symbols_in} {
        this->on_new_symbols_added();
    }

    size_t FactorTable::on_new_symbols_added() {
        // Do nothing if symbol table is up-to-date.
        if (this->entries.size() == this->symbols.size()) {
            return 0;
        }

        return this->check_for_new_factors();
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
            this->entries.emplace_back(symbol.Id());
            auto& entry = this->entries.back();

            // Check for factors
            entry.raw.sequences = this->context.factorize(symbol.sequence());

            // Check canonical form of factors
            entry.canonical.sequences.reserve(entry.raw.sequences.size());
            entry.canonical.symbols.reserve(entry.raw.sequences.size());

            for (const auto& factor_raw_seq : entry.raw.sequences) {
                entry.canonical.sequences.emplace_back(this->context.canonical_moment(factor_raw_seq));
                const auto& factor_seq = entry.canonical.sequences.back();

                // Try to find ID of (canonical) factor
                auto where = this->symbols.where(factor_seq);
                if (where != nullptr) {
                    assert(where->is_hermitian());
                    entry.canonical.symbols.emplace_back(where->Id());
                } else {
                    UniqueSequence us{factor_seq}; 
                    auto new_entry = this->symbols.merge_in(std::move(us));
                    entry.canonical.symbols.emplace_back(new_entry);
                }
            }
        }

        // Newly added symbols automatically should not factorize, and will be canonical
        const auto extra_symbols = static_cast<symbol_name_t>(this->symbols.size());
        for (symbol_name_t symbol_index = up_to_id; symbol_index < extra_symbols; ++symbol_index) {
            const auto& symbol = this->symbols[symbol_index];
            this->entries.emplace_back(symbol.Id());
            auto& entry = this->entries.back();
            entry.id = symbol.Id();
            entry.raw.sequences = std::vector<OperatorSequence>{symbol.sequence()};
            entry.canonical.sequences = std::vector<OperatorSequence>{symbol.sequence()};
            entry.canonical.symbols = std::vector<symbol_name_t>{entry.id};
        }

        return extra_symbols - next_id;
    }

}