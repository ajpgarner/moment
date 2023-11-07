/**
 * operator_rule.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "hashed_sequence.h"
#include "symbolic/monomial.h"

#include <iosfwd>
#include <optional>
#include <set>
#include <stdexcept>
#include <vector>

namespace Moment::Algebraic {

    namespace errors {
        class bad_hint : public std::logic_error {
        public:
            bad_hint();
        };

        class invalid_rule : public std::logic_error {
        public:
            explicit invalid_rule(const std::string& why);
        };

    }

    class AlgebraicPrecontext;
    class OperatorRulebook;

    class OperatorRule {
    public:
        using iter_t = sequence_storage_t::iterator;
        using const_iter_t = sequence_storage_t::const_iterator;

    private:
        HashedSequence rawLHS;
        HashedSequence rawRHS;
        bool is_trivial = false;
        bool map_to_zero = false;
        ptrdiff_t delta = 0;

    public:
        OperatorRule(HashedSequence lhs, HashedSequence rhs);

        OperatorRule(const OperatorRule& rhs) = default;

        OperatorRule(OperatorRule&& rhs) = default;

        /** The sequence on the left-hand side of the rule (pattern to match) */
        [[nodiscard]] const HashedSequence& LHS() const noexcept { return this->rawLHS; }

        /** The sequence on the right-hand side of the rule (replacement pattern) */
        [[nodiscard]] const HashedSequence& RHS() const noexcept { return this->rawRHS; }

        /** Iterator to the first place in the supplied sequence that matches the left-hand-side pattern of rule */
        [[nodiscard]] inline const_iter_t matches_anywhere(const_iter_t iter, const_iter_t iter_end) const noexcept {
            return this->rawLHS.matches_anywhere(iter, iter_end);
        }

        [[nodiscard]] sequence_storage_t
        apply_match_with_hint(const sequence_storage_t& input, const_iter_t hint) const;

         /**
         * Forms a new rule by combining overlapping left-hand-sides of rules (suffix of this w/ prefix of other), and
         * applying both rules to the joint string.
         * @param other The rule to combine this with.
         * @param hasher The hash function, for generating new rule.
         * @return The new rule, if overlap is nonzero, empty otherwise.
         */
        [[nodiscard]] std::optional<OperatorRule> combine(const OperatorRule& other,
                                                          const AlgebraicPrecontext& precontext) const;

        /**
         * True, if this rule directly implies the supplied other rule.
         */
        [[nodiscard]] bool implies(const OperatorRule& other) const noexcept;

        /** The amount the string-length changes by, on a successful match */
        [[nodiscard]] ptrdiff_t Delta() const noexcept { return this->delta; }

        /** True, if the rule is of the form A = A. */
        [[nodiscard]] bool trivial() const noexcept { return this->is_trivial; }

        /** The sign change induced by this rule, if any. */
        [[nodiscard]] constexpr SequenceSignType rule_sign() const noexcept {
            return this->rawRHS.get_sign();
        }

        /** True if the rule implies the LHS is equal to zero. */
        [[nodiscard]] bool implies_zero() const noexcept {
            return this->map_to_zero;
        }

        /** Forms a rule by conjugating both sides of the equality */
        [[nodiscard]] OperatorRule conjugate(const AlgebraicPrecontext& precontext) const;

        friend std::ostream& operator<<(std::ostream& os, const OperatorRule& msr);

        friend class OperatorRulebook;
    };
}