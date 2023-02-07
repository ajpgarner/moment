/**
 * rule_book.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "monomial_substitution_rule.h"
#include "algebraic_precontext.h"

#include "utilities/shortlex_hasher.h"

#include <iosfwd>
#include <map>
#include <vector>

namespace Moment::Algebraic {
    class RuleBook;

    class RuleLogger {
    public:
        virtual ~RuleLogger() = default;

        virtual void rule_reduced(const MonomialSubstitutionRule& old_rule,
                                      const MonomialSubstitutionRule& new_rule) = 0;

        virtual void rule_removed(const MonomialSubstitutionRule& ex_rule) = 0;

        virtual void rule_introduced(
                const MonomialSubstitutionRule& parent_rule_a,
                const MonomialSubstitutionRule& parent_rule_b,
                const MonomialSubstitutionRule& new_rule) = 0;

        virtual void rule_introduced(
                const MonomialSubstitutionRule& new_rule) = 0;

        virtual void rule_introduced_conjugate(
                const MonomialSubstitutionRule& parent_rule,
                const MonomialSubstitutionRule& new_rule) = 0;

        virtual void success(const RuleBook& rb, size_t attempts) = 0;

        virtual void failure(const RuleBook& rb, size_t attempts) = 0;
    };


    class RuleBook {
    public:
        using rule_map_t = std::map<size_t, MonomialSubstitutionRule>;

    private:
        AlgebraicPrecontext precontext;

        rule_map_t monomialRules{};

        bool is_hermitian = true;

    public:
        RuleBook(const AlgebraicPrecontext& precontext,
                 const std::vector<MonomialSubstitutionRule>& rules,
                 bool hermitian = true);

        explicit RuleBook(const AlgebraicPrecontext& pc, bool hermitian = true)
            : RuleBook(pc, std::vector<MonomialSubstitutionRule>{}, hermitian) { }

        /** Add rules */
        ptrdiff_t add_rules(const std::vector<MonomialSubstitutionRule>& rules, RuleLogger * logger = nullptr);

        /** Add single rule */
        ptrdiff_t add_rule(const MonomialSubstitutionRule& rule, RuleLogger * logger = nullptr);


        /** Handle to rules map. */
        [[nodiscard]] const auto& rules() const noexcept { return this->monomialRules; }

        /**
         * Number of rules in rule book.
         */
        [[nodiscard]]  size_t size() const noexcept { return this->monomialRules.size(); }

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

        /**
         * Reduce sequence, to best of knowledge, using rules
         * @param input The sequence to reduce
         * @return First: Reduced sequence. Second: True if sequence should be negated.
         */
        [[nodiscard]] std::pair<HashedSequence, bool> reduce(const HashedSequence& input) const;

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
        bool try_new_combination(RuleLogger * logger = nullptr);

        /**
         * Attempt to conjugate all rules in the set, reducing after each non-trivial conjugation.
         * @param logger Pointer (may be null) to class logging which new rules are introduced.
         * @param mock Set to true to not alter rule set, but just indicate existence of non-trivial conjugate.
         * @return Number of introduced rules.
         */
        size_t conjugate_ruleset(bool mock = false, RuleLogger * logger = nullptr);

        /**
         * Attempts to introduce a rule by conjugating the supplied input rule.
         * @param rule Reference to the rule to conjugate
         * @param mock Set to true to not alter rule set, but just return existence of non-trivial conjugate.
         * @param logger Pointer (may be null) to class logging which new rule is deduced.
         * @return True, if conjugate was non-trivial (and hence added to set).
         */
        bool try_conjugation(const MonomialSubstitutionRule& rule, bool mock = false, RuleLogger * logger = nullptr);

        /**
         * Print out rules.
         */
        friend std::ostream& operator<<(std::ostream& os, const RuleBook& rulebook);

        /**
         * Generate complete commutation rule list.
         * @param hasher For calculating hashes of generated strings
         * @param operator_count Number of operators
         * @return Vector of commutation rules.
         */
        static std::vector<MonomialSubstitutionRule> commutator_rules(const AlgebraicPrecontext& apc);

    };

}