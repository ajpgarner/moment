/**
 * operator_rulebook.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "operator_rule.h"
#include "algebraic_precontext.h"

#include "utilities/shortlex_hasher.h"

#include <iosfwd>
#include <map>
#include <vector>

namespace Moment::Algebraic {
    class OperatorRulebook;

    class RuleLogger {
    public:
        virtual ~RuleLogger() = default;

        virtual void rule_reduced(const OperatorRule& old_rule,
                                      const OperatorRule& new_rule) = 0;

        virtual void rule_removed(const OperatorRule& ex_rule) = 0;

        virtual void rule_introduced(
                const OperatorRule& parent_rule_a,
                const OperatorRule& parent_rule_b,
                const OperatorRule& new_rule) = 0;

        virtual void rule_introduced(
                const OperatorRule& new_rule) = 0;

        virtual void rule_introduced_conjugate(
                const OperatorRule& parent_rule,
                const OperatorRule& new_rule) = 0;

        virtual void success(const OperatorRulebook& rb, size_t attempts) = 0;

        virtual void failure(const OperatorRulebook& rb, size_t attempts) = 0;
    };

    enum class OperatorType {
        Generic,
        Normal,
        Hermitian
    };

    class OperatorRulebook {
    public:
        using rule_map_t = std::map<size_t, OperatorRule>;

    private:
        AlgebraicPrecontext precontext;

        rule_map_t monomialRules{};

        bool is_hermitian = true;

    public:
        OperatorRulebook(const AlgebraicPrecontext& precontext,
                         const std::vector<OperatorRule>& rules);

        explicit OperatorRulebook(const AlgebraicPrecontext& pc)
            : OperatorRulebook(pc, std::vector<OperatorRule>{}) { }

        /** Add rules */
        ptrdiff_t add_rules(const std::vector<OperatorRule>& rules, RuleLogger * logger = nullptr);

        /** Add single rule */
        ptrdiff_t add_rule(const OperatorRule& rule, RuleLogger * logger = nullptr);


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
         * Tests if the rule set has no critical pairs and is hence complete.
         * @param test_cc Also see if rule-set misses complexly-conjugated rules
         */
        [[nodiscard]] bool is_complete(bool test_cc = true) const;

        /**
         * Reduce sequence, to best of knowledge, using rules
         * @param input The sequence to reduce
         * @return First: Reduced sequence. Second: True if sequence should be negated.
         */
        [[nodiscard]] std::pair<HashedSequence, bool> reduce(const HashedSequence& input) const;

        /** Reduce rule, to best of knowledge, using rules in set */
        [[nodiscard]] OperatorRule reduce(const OperatorRule& input) const;

        /** True, if the supplied operator sequence could be reduced by a rule in the set */
        [[nodiscard]] bool can_reduce(const sequence_storage_t& input) const;

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
         * True if non-trivial rules can be formed through complex conjugation.
         */
        bool mock_conjugate() const;
        /**
         * Attempt to conjugate all rules in the set, reducing after each non-trivial conjugation.
         * @param mock Set to true to not alter rule set, but just indicate existence of non-trivial conjugate.
         * @param logger Pointer (may be null) to class logging which new rules are introduced.
         * @return Number of introduced rules.
         */
        size_t conjugate_ruleset(RuleLogger * logger = nullptr);

        /**
         * Attempts to introduce a rule by conjugating the supplied input rule.
         * @param rule Reference to the rule to conjugate
         * @param mock Set to true to not alter rule set, but just return existence of non-trivial conjugate.
         * @param logger Pointer (may be null) to class logging which new rule is deduced.
         * @return True, if conjugate was non-trivial (and hence added to set).
         */
        bool try_conjugation(const OperatorRule& rule, RuleLogger * logger = nullptr);

        /**
         * Print out rules.
         */
        friend std::ostream& operator<<(std::ostream& os, const OperatorRulebook& rulebook);

        /**
         * Generate complete commutation rule list.
         * @param apc Pre-context, for generating hashes and conjugates
         * @param output The vector of rules to append to - will have commutation rules added.
         */
        inline static std::vector<OperatorRule> commutator_rules(const AlgebraicPrecontext& apc) {
            std::vector<OperatorRule> output;
            OperatorRulebook::commutator_rules(apc, output);
            return output;
        }

        /**
         * Generate complete commutation rule list.
         * @param apc Pre-context, for generating hashes and conjugates
         * @return Vector of commutation rules.
         */
        static void commutator_rules(const AlgebraicPrecontext& apc, std::vector<OperatorRule>& output);

        /**
         * Generate "normal" rule list (a*a -> aa*), for non-self adjoint systems.
         * @param apc Pre-context, for generating hashes and conjugates.
         * @return Vector of normal rules; will be empty if apc is already self-adjoint.
         */
        static inline std::vector<OperatorRule> normal_rules(const AlgebraicPrecontext& apc) {
            std::vector<OperatorRule> output;
            OperatorRulebook::normal_rules(apc, output);
            return output;
        }

        /**
         * Generate "normal" rule list (a*a -> aa*), for non-self adjoint systems.
         * @param apc Pre-context, for generating hashes and conjugates.
         * @param output The vector of rules to append to - will have normal rules added (no change if apc is self-adj.)
         */
        static void normal_rules(const AlgebraicPrecontext& apc, std::vector<OperatorRule>& output);

    };

}