/**
 * rule_book.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "monomial_substitution_rule.h"

#include "operators/shortlex_hasher.h"

#include <iosfwd>
#include <map>
#include <vector>

namespace NPATK {
    class AlgebraicContext;

    class RuleLogger {
    public:
        virtual ~RuleLogger() = default;

        virtual void log_rule_reduced(const MonomialSubstitutionRule& old_rule,
                                      const MonomialSubstitutionRule& new_rule) = 0;

        virtual void log_rule_removed(const MonomialSubstitutionRule& ex_rule) = 0;

        virtual void log_rule_introduced(
                const MonomialSubstitutionRule& parent_rule_a,
                const MonomialSubstitutionRule& parent_rule_b,
                const MonomialSubstitutionRule& new_rule) = 0;
    };

    class OStreamRuleLogger : public RuleLogger {
    private:
        std::ostream& os;
    public:
        explicit OStreamRuleLogger(std::ostream& stream) : os{stream} { }

        void log_rule_reduced(const MonomialSubstitutionRule& old_rule,
                              const MonomialSubstitutionRule& new_rule) override;

        void log_rule_removed(const MonomialSubstitutionRule& ex_rule) override;

        void log_rule_introduced(
                const MonomialSubstitutionRule& parent_rule_a,
                const MonomialSubstitutionRule& parent_rule_b,
                const MonomialSubstitutionRule& new_rule) override;
    };

    class RuleBook {
    public:
        using rule_map_t = std::map<size_t, MonomialSubstitutionRule>;
    private:
        const ShortlexHasher& hasher;

        rule_map_t monomialRules;

    public:
        RuleBook(const ShortlexHasher& hasher, const std::vector<MonomialSubstitutionRule>& rules);

        explicit RuleBook(const ShortlexHasher& hasher)
            : RuleBook(hasher, std::vector<MonomialSubstitutionRule>{}) { }

        /** Handle to rules */
        [[nodiscard]] const auto& rules() const noexcept { return this->monomialRules; }

        /**
         * Attempts, using Knuth-Bendix algorithm, to complete the rule sets.
         * @param max_iterations The maximum number of new non-trivial reductions deduced before giving up.
         * @param logger Pointer (may be null) to class logging the completion attempt.
         * @return True, if ruleset is complete (i.e. no new reductions can be deduced).
         */
        bool complete(size_t max_iterations, RuleLogger * logger = nullptr);

        /**
         * Tests if the rule set has no critical pairs and is hence complete
         */
        [[nodiscard]] bool is_complete() const;

        /** Reduce sequence, to best of knowledge, using rules */
        [[nodiscard]] HashedSequence reduce(const HashedSequence& input) const;

        /** Reduce rule, to best of knowledge, using rules in set */
        [[nodiscard]] MonomialSubstitutionRule reduce(const MonomialSubstitutionRule& input) const;

        /**
         * Simplify any rules in the set that can be reduced by other rules.
         * @param logger Pointer (may be null) to class logging which rules are reduced.
         * @return Number of changed rules.
         */
        size_t reduce_ruleset(RuleLogger * logger = nullptr);

        /**
         * Attempt to deduce a novel and non-trivial rule from considering overlaps within ruleset (Knuth-Bendix).
         * @param logger Pointer (may be null) to class logging which new rule is deduced.
         * @return True, if a non-trivial rule was found.
         */
        bool try_new_reduction(RuleLogger * logger = nullptr);

    };

}