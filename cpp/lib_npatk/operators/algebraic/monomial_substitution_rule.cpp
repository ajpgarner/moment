/**
 * monomial_substitution_rule.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "monomial_substitution_rule.h"

#include <cassert>

#include <algorithm>
#include <iostream>
#include <iterator>

namespace NPATK {
    namespace errors {
        bad_hint::bad_hint()
            : std::logic_error("Hint supplied does not match rule.") {
        }

        invalid_rule::invalid_rule(const std::string &why)
            : std::logic_error("Invalid rule: " + why) {
        }
    }

    std::ostream& operator<<(std::ostream& os, const MonomialSubstitutionRule& msr) {
        if (msr.rawLHS.empty()) {
            os << "I";
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
            os << "I";
        } else {
            for (const auto i : msr.rawRHS) {
                os << "X" << i;
            }
        }

        return os;
    }

    MonomialSubstitutionRule::MonomialSubstitutionRule(HashedSequence lhs,
                                                       HashedSequence rhs,
                                                       bool negated)
            : rawLHS{std::move(lhs)}, rawRHS{std::move(rhs)}, is_negated{negated},
              is_trivial{(lhs.hash() == rhs.hash()) && !negated},
              delta{static_cast<ptrdiff_t>(rawRHS.size()) - static_cast<ptrdiff_t>(rawLHS.size())} {
        if (rawLHS < rawRHS) {
            throw errors::invalid_rule(std::string("Rule was not a reduction: ")
                                       + "the RHS must not exceed LHS in shortlex ordering.");
        }
    }

    std::vector<oper_name_t>
    MonomialSubstitutionRule::apply_match_with_hint(const std::vector<oper_name_t>& input,
                                                    const_iter_t hint) const {

        // Reserve vector, return empty vector, or give error:
        ptrdiff_t new_size = static_cast<ptrdiff_t>(input.size()) + this->delta;
        if (new_size <= 0) {
            if (new_size < 0) {
                throw errors::bad_hint{};
            }
            return std::vector<oper_name_t>{};
        }
        std::vector<oper_name_t> output;
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

    bool MonomialSubstitutionRule::implies(const MonomialSubstitutionRule &other) const noexcept {
        // First, do we find LHS in other rule?
        auto embeddedLHS_begin = this->rawLHS.matches_anywhere(other.rawLHS.begin(), other.rawLHS.end());
        if (embeddedLHS_begin== other.rawLHS.end()) {
            return false;
        }

        // Second, do we find RHS in other rule?
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

    std::optional<MonomialSubstitutionRule>
    MonomialSubstitutionRule::combine(const MonomialSubstitutionRule &other, const ShortlexHasher& hasher) const {
        // First, do we have overlap? If not, early exit.
        ptrdiff_t overlap_size = this->LHS().suffix_prefix_overlap(other.rawLHS);
        if (overlap_size <= 0) {
            return std::nullopt;
        }

        // Next, make merged string from both rules' LHS
        std::vector<oper_name_t> joined_string;
        joined_string.reserve(static_cast<ptrdiff_t>(this->rawLHS.size() + other.rawLHS.size())
                              - overlap_size);
        std::copy(this->rawLHS.begin(), this->rawLHS.end() - static_cast<ptrdiff_t>(overlap_size),
                  std::back_inserter(joined_string));
        std::copy(other.rawLHS.begin(), other.rawLHS.end(),
                  std::back_inserter(joined_string));

        // Apply this rule to joint string
        auto rawViaThis = this->apply_match_with_hint(joined_string, joined_string.begin());
        auto rawHashThis = hasher(rawViaThis);

        // Apply other rule to joint string
        auto rawViaOther = other.apply_match_with_hint(joined_string,
                                                       joined_string.cend()
                                                            - static_cast<ptrdiff_t>(other.rawLHS.size()));
        auto rawHashOther = hasher(rawViaOther);

        // Negative if only one rule involves negation
        bool negation = this->is_negated != other.is_negated;

        // Orient rules and return
        if (rawHashThis < rawHashOther) {
            return MonomialSubstitutionRule{HashedSequence{std::move(rawViaOther), rawHashOther},
                                            HashedSequence{std::move(rawViaThis), rawHashThis}, negation};
        } else {
            return MonomialSubstitutionRule{HashedSequence{std::move(rawViaThis), rawHashThis},
                                            HashedSequence{std::move(rawViaOther), rawHashOther}, negation};
        }
    }

    MonomialSubstitutionRule MonomialSubstitutionRule::conjugate(const ShortlexHasher& hasher) const {
        auto lhs = this->rawLHS.conjugate(hasher);
        auto rhs = this->rawRHS.conjugate(hasher);
        if (lhs < rhs) {
            return MonomialSubstitutionRule(std::move(rhs), std::move(lhs), this->is_negated);
        } else {
            return MonomialSubstitutionRule(std::move(lhs), std::move(rhs), this->is_negated);
        }
    }

}