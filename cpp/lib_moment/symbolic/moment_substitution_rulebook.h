/**
 * moment_substitution_rulebook.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_substitution_rule.h"

namespace Moment {
    class SymbolTable;

    class MomentSubstitutionRulebook {
    public:
        /**
         * Order first by operator hash of forward sequence, then by conjugation sequence.
         * This is not quite the same as op-hash order; because it guarantees complex conjugate strings are adjacent.
         */
        struct CompareByOpHash {
        public:
            const SymbolTable& symbolTable;

            explicit CompareByOpHash(const SymbolTable& symbolTable)
                    : symbolTable{symbolTable} { }

            [[nodiscard]] bool operator()(const SymbolExpression& lhs, const SymbolExpression& rhs) const noexcept;
        };

    private:
        const SymbolTable& symbols;
        const CompareByOpHash comparator;

        std::map<symbol_name_t, MomentSubstitutionRule> rules;

        std::vector<SymbolCombo> raw_rules;

    public:
        explicit MomentSubstitutionRulebook(const SymbolTable& table);

        void add_raw_rules(std::vector<SymbolCombo>&& raw);

        bool complete();

    };
}