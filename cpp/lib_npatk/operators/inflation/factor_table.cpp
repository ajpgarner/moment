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
        this->on_new_symbols_added();
    }

    size_t FactorTable::on_new_symbols_added() {
        // Do nothing if symbol table is up-to-date.
        if (this->entries.size() == this->symbols.size()) {
            return 0;
        }

        const auto previous_size = static_cast<symbol_name_t>(this->entries.size());
        size_t symbols_added = this->check_for_new_factors();

        const size_t extra_symbols = this->check_and_link_canonical_moments(previous_size);

        // Second pass, if new symbols introduced
        if (extra_symbols > 0) {
            const auto size_after_first_pass =  static_cast<symbol_name_t>(this->entries.size());
            // Second pass on factors
            symbols_added += this->check_for_new_factors();

            const size_t should_be_zero = this->check_and_link_canonical_moments(size_after_first_pass);
            assert(should_be_zero == 0);
        }

        return symbols_added;
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
            this->entries.emplace_back(symbol.Id(), context.canonical_moment(symbol.sequence()));
            auto& entry = this->entries.back();

            // Check for factors
            entry.raw.sequences = this->context.factorize(symbol.sequence());

            // Look up symbol IDs of factors
            entry.raw.symbols.reserve(entry.raw.sequences.size());
            for (const auto& factor_seq : entry.raw.sequences) {
                auto where = this->symbols.where(factor_seq);
                if (where != nullptr) {
                    assert(where->is_hermitian());
                    entry.raw.symbols.emplace_back(where->Id());
                } else {
                    UniqueSequence us{factor_seq, context.hash(factor_seq)};
                    auto new_entry = this->symbols.merge_in(std::move(us));
                    entry.raw.symbols.emplace_back(new_entry);
                }
            }
        }

        // Newly added symbols automatically will not factorize
        const auto extra_symbols = static_cast<symbol_name_t>(this->symbols.size());
        for (symbol_name_t symbol_index = up_to_id; symbol_index < extra_symbols; ++symbol_index) {
            const auto& symbol = this->symbols[symbol_index];
            this->entries.emplace_back(symbol.Id(), context.canonical_moment(symbol.sequence()));
            auto& entry = this->entries.back();
            entry.id = symbol.Id();
            entry.raw.sequences = std::vector<OperatorSequence>{symbol.sequence()};
            entry.raw.symbols = std::vector<symbol_name_t>{entry.id};
        }

        return extra_symbols - next_id;
    }

    size_t FactorTable::check_and_link_canonical_moments(const symbol_name_t from_id) {
        // Do nothing, if nothing to do.
        const size_t initial_table_size = this->entries.size();
        if (from_id == initial_table_size) {
             return 0;
        }

        // Look up symbols
        for (symbol_name_t symbol_index = from_id; symbol_index < initial_table_size; ++symbol_index) {
            auto& entry = this->entries[symbol_index];
            const auto* symbolInfoPtr = this->symbols.where(entry.canonical_form_sequence);
            if (nullptr != symbolInfoPtr) {
                entry.canonical_id = symbolInfoPtr->Id();
            } else {
                // Canonical variant doesn't previously exist in table, so add it.
                entry.canonical_id = this->symbols.merge_in(
                    UniqueSequence{entry.canonical_form_sequence, context.hash(entry.canonical_form_sequence)}
                );
            }
        }

        // Also look up canonical versions of factors
        for (symbol_name_t symbol_index = from_id; symbol_index < initial_table_size; ++symbol_index) {
            auto &entry = this->entries[symbol_index];
            entry.canonical.sequences.reserve(entry.raw.sequences.size());
            entry.canonical.symbols.reserve(entry.raw.symbols.size());

            for (const auto factor_sym_id : entry.raw.symbols) {
                const auto& factor_entry = this->entries[factor_sym_id];
                entry.canonical.sequences.emplace_back(factor_entry.canonical_form_sequence);
                entry.canonical.symbols.emplace_back(factor_entry.canonical_id);
            }
        }

        // Return if new symbols have been added in the course of identifying canonical moments
        return this->symbols.size() - initial_table_size;
    }
}