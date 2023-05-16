/**
 * moment_substitution_rulebook.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_substitution_rule.h"

#include <map>
#include <memory>
#include <vector>

namespace Moment {
    class SymbolTable;

    class MomentSubstitutionRulebook {
    public:
        const SymbolTable& symbols;

    private:
        std::map<symbol_name_t, MomentSubstitutionRule> rules;

        std::vector<SymbolCombo> raw_rules;

        std::unique_ptr<SymbolComboFactory> factory;

    public:
        explicit MomentSubstitutionRulebook(const SymbolTable& table)
            : MomentSubstitutionRulebook(table, std::make_unique<SymbolComboFactory>(table)) { }

        explicit MomentSubstitutionRulebook(const SymbolTable& table, std::unique_ptr<SymbolComboFactory> factory);

        void add_raw_rules(std::vector<SymbolCombo>&& raw);

        /**
         * Try to add an oriented rule directly.
         * @returns True if rule added, false if collision.
         */
        bool inject(MomentSubstitutionRule&& msr);

        /**
         * Process raw-rules into completed rule-set.
         * @return Number of rules added.
         */
        size_t complete();

        /**
         * Returns iterator to first rule that would reduce sequence.
         */
        [[nodiscard]] std::map<symbol_name_t, MomentSubstitutionRule>::const_reverse_iterator
        first_matching_rule(const SymbolCombo& combo) const noexcept;

        /**
         * Start reduction from hint onwards (will not apply any reductions before hint).
         */
        [[nodiscard]] SymbolCombo reduce_with_rule_hint(
                std::map<symbol_name_t, MomentSubstitutionRule>::const_reverse_iterator rule_hint,
                SymbolCombo combo) const;

        [[nodiscard]] SymbolCombo reduce(SymbolCombo combo) const {
            return this->reduce_with_rule_hint(this->rules.crbegin(), std::move(combo));
        }

        /**
         * True if supplied rule matches key already in rulebook.
         * (Complexity O(log(N)) where N are the number of rules.)
         */
        [[nodiscard]] bool collides(const MomentSubstitutionRule& msr) const noexcept;


        /**
         * True if supplied rule matches key at end of rulebook.
         * (Complexity O(1))
         */
        [[nodiscard]] bool collides_at_end(const MomentSubstitutionRule& msr) const noexcept;

        /**
         * True if no reduction rules.
         */
        [[nodiscard]] bool empty() const noexcept { return this->rules.empty(); }

        /**
         * Get number of reduction rules.
         */
        [[nodiscard]] size_t size() const noexcept { return this->rules.size(); }

        /**
         * Begin iteration over rules.
         */
        [[nodiscard]] auto begin() const noexcept { return this->rules.cbegin(); }

        /**
         * End iteration over rules.
         */
        [[nodiscard]] auto end() const noexcept { return this->rules.cend(); }

        /**
         * Return reference to associated SymbolComboFactory.
         */
        [[nodiscard]] const SymbolComboFactory& Factory() const noexcept { return *this->factory; }



    };
}