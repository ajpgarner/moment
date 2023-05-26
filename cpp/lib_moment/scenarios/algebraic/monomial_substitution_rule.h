/**
 * monomial_substitution_rule.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
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
    class RuleBook;

    class MonomialSubstitutionRule {
    public:
        using iter_t = sequence_storage_t::iterator;
        using const_iter_t = sequence_storage_t::const_iterator;

    private:
        HashedSequence rawLHS;
        HashedSequence rawRHS;
        bool is_negated = false;
        bool is_trivial = false;
        ptrdiff_t delta = 0;

    public:
        MonomialSubstitutionRule(HashedSequence lhs, HashedSequence rhs,
                                 bool negated = false);

        MonomialSubstitutionRule(const MonomialSubstitutionRule& rhs) = default;

        MonomialSubstitutionRule(MonomialSubstitutionRule&& rhs) = default;

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
        [[nodiscard]] std::optional<MonomialSubstitutionRule> combine(const MonomialSubstitutionRule& other,
                                                                      const AlgebraicPrecontext& precontext) const;

        /**
         * True, if this rule directly implies the supplied other rule.
         */
        [[nodiscard]] bool implies(const MonomialSubstitutionRule& other) const noexcept;

        /** The amount the string-length changes by, on a successful match */
        [[nodiscard]] ptrdiff_t Delta() const noexcept { return this->delta; }

        /** True, if the rule is of the form A = A. */
        [[nodiscard]] bool trivial() const noexcept { return this->is_trivial; }

        /** True, if the rule requires a negative sign */
        [[nodiscard]] bool negated() const noexcept { return this->is_negated; }

        /** True if the rule is of the form A = -A */
        [[nodiscard]] bool implies_zero() const noexcept {
            return this->is_negated && (this->rawLHS.hash() == this->rawRHS.hash());
        }

        /** Forms a rule by conjugating both sides of the equality */
        [[nodiscard]] MonomialSubstitutionRule conjugate(const AlgebraicPrecontext& precontext) const;


        friend std::ostream& operator<<(std::ostream& os, const MonomialSubstitutionRule& msr);

        friend class RuleBook;
    };
}