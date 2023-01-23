/**
 * factor_table.h
 * 
 * Copyright (c) 2022-2023 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "scenarios/operator_sequence.h"
#include "utilities/index_tree.h"

#include <set>
#include <string>
#include <vector>

namespace Moment {
    class SymbolTable;
    class SymbolicMatrix;
}

namespace Moment::Inflation {
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

            /** The number of times this symbol appears as a factor of another symbol */
            size_t appearances = 0;

        public:
            explicit FactorEntry(const symbol_name_t sym_id)
                : id{sym_id} { }

            /** True if table entry does not factorize */
            [[nodiscard]] bool fundamental() const noexcept { return canonical.sequences.size() <= 1; }

            [[nodiscard]] std::string sequence_string() const;
        };

    private:
        const InflationContext& context;
        SymbolTable& symbols;

        std::vector<FactorEntry> entries;

        IndexTree<symbol_name_t, symbol_name_t> index_tree;

    public:
        /** Create additional factor information, synchronized with symbol table. */
        explicit FactorTable(const InflationContext& context, SymbolTable& symbols);

        /** Bring factor table up to date, when new symbols are added to symbol table. */
        size_t on_new_symbols_added();

        /** The number of entries in the factor table. */
        [[nodiscard]] size_t size() const noexcept { return this->entries.size(); }

        /** True if factor table contains at least one entry. */
        [[nodiscard]] bool empty() const noexcept { return this->entries.empty(); }

        /** Iterator to factor entries */
        [[nodiscard]] auto begin() const noexcept { return this->entries.cbegin(); }

        /** Iterator end to factor entries */
        [[nodiscard]] auto end() const noexcept { return this->entries.cend(); }

        /** Access one entry in factor table by index. */
        [[nodiscard]] const FactorEntry& operator[](size_t index) const noexcept { return this->entries[index]; }

        /** Attempt to find entry by factors */
        [[nodiscard]] std::optional<symbol_name_t> find_index_by_factors(std::span<const symbol_name_t> factors) const {
            return this->index_tree.find(factors);
        }

        /** Attempt to find entry by factors (initializer list) */
        [[nodiscard]] std::optional<symbol_name_t>
        find_index_by_factors(std::initializer_list<symbol_name_t> factors) const {
            std::vector<symbol_name_t> factor_vec(factors);
            return this->index_tree.find(factor_vec);
        }

        /**
         * Manually insert a list of factors associated with an entry.
         */
        void register_new(symbol_name_t id, std::vector<symbol_name_t> factors);


    public:
        /** Merge and sort two sets of factors, removing redundant identities */
        [[nodiscard]] static std::vector<symbol_name_t>
        combine_symbolic_factors(std::vector<symbol_name_t> left, const std::vector<symbol_name_t>& right);

    private:
        size_t check_for_new_factors();
        
    };
}