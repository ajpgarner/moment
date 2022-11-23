/**
 * factor_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "operators/operator_sequence.h"

#include <vector>


namespace NPATK {
    class SymbolTable;
    class InflationContext;

    class FactorTable {
    public:
        struct FactorEntry {
            /** Identity, aligned with index in symbol table. */
            symbol_name_t id = -1;

            /** Equivalent operator sequence for purpose of moments (i.e. relabelling of source indices). */
            OperatorSequence canonical_form_sequence;

            /** Associated symbol with canonoical operator sequence, aligned with index in symbol table. */
            symbol_name_t canonical_id = -1;

            /** The factors, as they appear */
            struct RawFactors {
                std::vector<OperatorSequence> sequences{};
                std::vector<symbol_name_t> symbols{};
            } raw;

            /** Equivalent factors, when considered as moments (i.e. after relabelling of source indices) */
            struct CanonicalFactors {
                std::vector<OperatorSequence> sequences{};
                std::vector<symbol_name_t> symbols{};
            } canonical;

        public:
            FactorEntry(const symbol_name_t sym_id, OperatorSequence canonical_form)
                : id{sym_id}, canonical_form_sequence{std::move(canonical_form)} { }

            [[nodiscard]] bool is_canonical() const noexcept { return this->id == this->canonical_id; }
        };

    private:
        const InflationContext& context;
        SymbolTable& symbols;

        std::vector<FactorEntry> entries;

    public:
        explicit FactorTable(const InflationContext& context, SymbolTable& symbols);

        size_t on_new_symbols_added();

        [[nodiscard]] size_t size() const noexcept { return this->entries.size(); }

        [[nodiscard]] bool empty() const noexcept { return this->entries.empty(); }

        [[nodiscard]] const FactorEntry& operator[](size_t index) const noexcept { return this->entries[index]; }

    private:
        size_t check_for_new_factors();

        size_t check_and_link_canonical_moments(symbol_name_t from_id);



    };
}