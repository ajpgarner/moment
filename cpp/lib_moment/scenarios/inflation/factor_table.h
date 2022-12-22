/**
 * factor_table.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "scenarios/operator_sequence.h"

#include <string>
#include <vector>


namespace Moment {
    class SymbolTable;
    class InflationContext;

    class FactorTable {
    public:
        struct FactorEntry {
            /** Identity, aligned with index in symbol table. */
            symbol_name_t id = -1;

            /** The factors, as they appear */
            struct RawFactors {
                std::vector<OperatorSequence> sequences{};
            } raw;

            /** Equivalent factors, when considered as moments (i.e. after relabelling of source indices) */
            struct CanonicalFactors {
                std::vector<OperatorSequence> sequences{};
                std::vector<symbol_name_t> symbols{};
            } canonical;

        public:
            FactorEntry(const symbol_name_t sym_id)
                : id{sym_id} { }


            std::string sequence_string() const;
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



    };
}