/**
 * operator_rule.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_rule.h"
#include "algebraic_precontext.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <iterator>

namespace Moment::Algebraic {
    namespace errors {
        bad_hint::bad_hint()
            : std::logic_error("Hint supplied does not match rule.") {
        }

        invalid_rule::invalid_rule(const std::string &why)
            : std::logic_error("Invalid rule: " + why) {
        }
    }

    std::ostream& operator<<(std::ostream& os, const OperatorRule& msr) {
        if (msr.rawLHS.empty()) {
            if (msr.rawLHS.zero()) {
                os << "0";
            } else {
                os << "I";
            }
        } else {
            for (const auto i : msr.rawLHS) {
                os << "X" << i;
            }
        }

        os << " -> ";
        if (msr.negated()) {
            os << "-";
        }

        if (msr.rawRHS.empty()) {
            if (msr.rawRHS.zero()) {
                os << "0";
            } else {
                os << "I";
            }
        } else {
            for (const auto i : msr.rawRHS) {
                os << "X" << i;
            }
        }

        return os;
    }

    OperatorRule::OperatorRule(HashedSequence lhs,
                               HashedSequence rhs)
            : rawLHS{std::move(lhs)}, rawRHS{std::move(rhs)},
              map_to_zero(rhs.zero()),
              delta{static_cast<ptrdiff_t>(rawRHS.size()) - static_cast<ptrdiff_t>(rawLHS.size())} {

        // Move negation to RHS
        if (this->rawLHS.negated()) {
            this->rawLHS.set_negation(false);
            this->rawRHS.set_negation(!rhs.negated());
        }

        // Determine if rule is trivial
        this->is_trivial = (this->rawLHS.hash() == this->rawRHS.hash()) && !this->rawRHS.negated();

        // Check rule is a reduction
        if (rawLHS.hash() < rawRHS.hash()) {
            throw errors::invalid_rule(std::string("Rule was not a reduction: ")
                                       + "the RHS must not exceed LHS in shortlex ordering.");
        }
    }

    sequence_storage_t
    OperatorRule::apply_match_with_hint(const sequence_storage_t& input,
                                        const_iter_t hint) const {
        // If map to zero, return empty sequence.
        if (this->map_to_zero) {
            return sequence_storage_t{};
        }

        // Reserve vector, return empty vector, or give error:
        ptrdiff_t new_size = static_cast<ptrdiff_t>(input.size()) + this->delta;
        if (new_size <= 0) {
            if (new_size < 0) {
                throw errors::bad_hint{};
            }
            return sequence_storage_t{};
        }
        sequence_storage_t output;
        output.reserve(new_size);

        // Copy start of input string up to hint
        std::copy(input.cbegin(), hint, std::back_inserter(output));

        // Copy substituted string
        std::copy(rawRHS.begin(), rawRHS.end(), std::back_inserter(output));

        // Copy remainder of input string
        hint += static_cast<ptrdiff_t>(rawLHS.size());
        std::copy(hint, input.cend(), std::back_inserter(output));

        // Post-condition sanity check
        if (output.size() != new_size) {
            throw errors::bad_hint();
        }

        return output;
    }

    bool OperatorRule::implies(const OperatorRule &other) const noexcept {
        // First, do we find LHS in other rule LHS?
        auto embeddedLHS_begin = this->rawLHS.matches_anywhere(other.rawLHS.begin(), other.rawLHS.end());
        if (embeddedLHS_begin== other.rawLHS.end()) {
            return false;
        }

        // Second, do we find RHS in other rule RHS?
        auto embeddedRHS_begin = this->rawRHS.matches_anywhere(other.rawRHS.begin(), other.rawRHS.end());
        if (embeddedRHS_begin == other.rawRHS.end()) {
            return false;
        }

        // Check that prefix of other rule matches
        if (!std::equal(other.rawLHS.begin(), embeddedLHS_begin,
                        other.rawRHS.begin(), embeddedRHS_begin)) {
            return false;
        }

        // Check that suffix of other rule matches
        auto suffix_LHS = embeddedLHS_begin + static_cast<ptrdiff_t>(this->rawLHS.size());
        auto suffix_RHS = embeddedRHS_begin + static_cast<ptrdiff_t>(this->rawRHS.size());
        if (!std::equal(suffix_LHS, other.rawLHS.end(),
                        suffix_RHS, other.rawRHS.end())) {
            return false;
        }

        // No mismatches
        return true;
    }

    std::optional<OperatorRule>
    OperatorRule::combine(const OperatorRule &other, const AlgebraicPrecontext& pc) const {
        // First, do we have overlap? If not, early exit.
        ptrdiff_t overlap_size = this->LHS().suffix_prefix_overlap(other.rawLHS);
        if (overlap_size <= 0) {
            return std::nullopt;
        }

        // Next, make merged string from both rules' LHS
        sequence_storage_t joined_string;
        joined_string.reserve(static_cast<ptrdiff_t>(this->rawLHS.size() + other.rawLHS.size())
                              - overlap_size);
        std::copy(this->rawLHS.begin(), this->rawLHS.end() - static_cast<ptrdiff_t>(overlap_size),
                  std::back_inserter(joined_string));
        std::copy(other.rawLHS.begin(), other.rawLHS.end(),
                  std::back_inserter(joined_string));

        // Apply this rule to joint string
        auto rawViaThis = this->apply_match_with_hint(joined_string, joined_string.begin());
        auto rawHashThis = this->implies_zero() ? 0 : pc.hash(rawViaThis);

        // Apply other rule to joint string
        auto rawViaOther = other.apply_match_with_hint(joined_string,
                                                       joined_string.cend()
                                                        - static_cast<ptrdiff_t>(other.rawLHS.size()));
        auto rawHashOther = other.implies_zero() ? 0 : pc.hash(rawViaOther);

        // Negative if only one rule involves negation (and we are not setting to zero).
        const bool implies_zero = (rawHashThis == 0) || (rawHashOther == 0);
        const bool negation = !implies_zero && (this->negated() != other.negated());

        // Orient rules and return
        if (rawHashThis < rawHashOther) {
            return OperatorRule{HashedSequence{std::move(rawViaOther), rawHashOther},
                                HashedSequence{std::move(rawViaThis), rawHashThis, negation}};
        } else {
            return OperatorRule{HashedSequence{std::move(rawViaThis), rawHashThis},
                                HashedSequence{std::move(rawViaOther), rawHashOther, negation}};
        }
    }

    OperatorRule OperatorRule::conjugate(const AlgebraicPrecontext& pc) const {
        auto lhs = pc.conjugate(this->rawLHS);
        auto rhs = pc.conjugate(this->rawRHS);
        if (lhs < rhs) {
            return OperatorRule{std::move(rhs), std::move(lhs)};
        } else {
            return OperatorRule{std::move(lhs), std::move(rhs)};
        }
    }

}