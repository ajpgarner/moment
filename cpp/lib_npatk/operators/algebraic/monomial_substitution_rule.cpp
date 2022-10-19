/**
 * monomial_substitution_rule.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "monomial_substitution_rule.h"

#include "raw_sequence_book.h"

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

    MonomialSubstitutionRule::MonomialSubstitutionRule(HashedSequence lhs,
                                                       HashedSequence rhs,
                                                       bool negated)
            : rawLHS{std::move(lhs)}, rawRHS{std::move(rhs)}, negated{negated},
              Delta{static_cast<ptrdiff_t>(rawRHS.size()) - static_cast<ptrdiff_t>(rawLHS.size())} {
        if (rawLHS < rawRHS) {
            throw errors::invalid_rule(std::string("Rule was not a reduction: ")
                                       + " the RHS must not exceed LHS in shortlex ordering.");
        }
    }

    std::vector<oper_name_t>
    MonomialSubstitutionRule::apply_match_with_hint(const std::vector<oper_name_t>& input,
                                                    const_iter_t hint) const {

        // Reserve vector, return empty vector, or give error:
        ptrdiff_t new_size = static_cast<ptrdiff_t>(input.size()) + this->Delta;
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

    size_t MonomialSubstitutionRule::all_matches(std::vector<SymbolPair>& output,
                                                 const RawSequenceBook& rsb,
                                                 const RawSequence& input_sequence) const {
        assert(rsb.where(input_sequence) != nullptr);
        assert(input_sequence.size() <= rsb.longest_sequence());

        size_t match_count = 0;
        auto input_iter = input_sequence.begin();
        auto match_iter = this->matches_anywhere(input_iter, input_sequence.end());
        while (match_iter != input_sequence.end()) {
            auto altered_string = this->apply_match_with_hint(input_sequence.raw(), match_iter);
            const auto *target_seq = rsb.where(altered_string);
            if (target_seq == nullptr) {
                throw std::logic_error{"Internal error: Substitution resulted in illegal string!"};
            }

            // Register symbol link
            output.emplace_back(input_sequence.raw_id, target_seq->raw_id, this->negated, false);

            // Find next match
            match_iter = this->matches_anywhere(match_iter + 1, input_sequence.end());
            ++match_count;
        }
        return match_count;
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
        if (msr.negated) {
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

}