/**
 * factor_table.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "factor_table.h"

#include "inflation_context.h"

#include "symbolic/symbol_table.h"

#include <algorithm>
#include <sstream>

namespace Moment::Inflation {

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

            // Get symbols for each canonical sequence
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

            // Canonical symbols should be sorted in factor entry
            std::sort(entry.canonical.symbols.begin(), entry.canonical.symbols.end());
            // Add to index tree
            this->index_tree.add(entry.canonical.symbols, entry.id);
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
            this->index_tree.add(entry.canonical.symbols, entry.id);
        }

        // Count factors
        for (size_t entry_index = next_id; entry_index < extra_symbols; ++entry_index) {
            const auto& factor_entry = this->entries[entry_index];
            // Non-trivial factors?
            if (factor_entry.canonical.symbols.size() > 1) {
                for (const auto& factor_symbol : factor_entry.canonical.symbols) {
                    assert(factor_symbol < this->entries.size());
                    this->entries[factor_symbol].appearances += 1;
                }
            }
        }

        return extra_symbols - next_id;
    }

    void FactorTable::register_new(symbol_name_t id, std::vector<symbol_name_t> factors) {
        this->entries.emplace_back(id);
        assert(this->entries.size() == id+1);
        auto& new_entry = this->entries.back();
        new_entry.canonical.symbols.swap(factors);

        // Look up associated sequences
        new_entry.canonical.sequences.reserve(new_entry.canonical.symbols.size());
        for (auto sym_id : new_entry.canonical.symbols) {
            new_entry.canonical.sequences.push_back(symbols[sym_id].sequence());
        }

        // Create index
        index_tree.add(new_entry.canonical.symbols, new_entry.id);

    }

    std::vector<symbol_name_t> FactorTable::combine_symbolic_factors(std::vector<symbol_name_t> left,
                                                                     const std::vector<symbol_name_t>& right) {
        // First, no factors on either side -> identity.
        if (left.empty() && right.empty()) {
            return {1};
        }

        // Copy, and sort factors
        std::vector<symbol_name_t> output{std::move(left)};
        output.reserve(output.size() + right.size());
        output.insert(output.end(), right.cbegin(), right.cend());
        std::sort(output.begin(), output.end());

        // If "0" is somehow a factor of either left or right, the product is zero
        assert(!output.empty());
        if (output[0] == 0) {
            [[unlikely]]
            return {0};
        }

        // Now, if we have more than one factor, prune identities (unless only identity)
        if (output.size()>1) {
            auto first_non_id = std::upper_bound(output.begin(), output.end(), 1);
            output.erase(output.begin(), first_non_id);
            // Factors were  1 x 1 x ... x 1
            if (output.empty()) {
                return {1};
            }
        }
        return output;
    }

}