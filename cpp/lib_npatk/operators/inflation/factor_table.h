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
            symbol_name_t id = -1;
            std::vector<OperatorSequence> sequences{};
            std::vector<symbol_name_t> symbols{};
        };

    private:
        const InflationContext& context;
        SymbolTable& symbols;

        std::vector<FactorEntry> entries;

    public:
        explicit FactorTable(const InflationContext& context, SymbolTable& symbols);

        /**
         * Construct new factors, adding new symbols to SymbolTable if necessary.
         */
        size_t check_for_new_factors();

        [[nodiscard]] size_t size() const noexcept { return this->entries.size(); }

        [[nodiscard]] bool empty() const noexcept { return this->entries.empty(); }

        [[nodiscard]] const FactorEntry& operator[](size_t index) const noexcept { return this->entries[index]; }

    };
}