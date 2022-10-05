/**
 * monomial_substitution_rule.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "monomial_substitution_rule.h"

#include "raw_sequence_book.h"

#include <cassert>

#include <algorithm>
#include <iterator>

namespace NPATK {

    namespace errors {
        bad_hint::bad_hint()
            : std::logic_error("Hint supplied does not match rule.") {

        }
    }

    bool MonomialSubstitutionRule::matches(MonomialSubstitutionRule::const_iter_t test_iter,
                                           const MonomialSubstitutionRule::const_iter_t test_iter_end) const noexcept {
        // No match, if not enough space left
        if (std::distance(test_iter, test_iter_end) < this->rawLHS.size()) {
            return false;
        }

        auto this_iter = this->rawLHS.cbegin();
        const auto this_iter_end = this->rawLHS.cend();
        while ((test_iter != test_iter_end) && (this_iter != this_iter_end)) {
            if (*this_iter != *test_iter) {
                return false;
            }
            ++test_iter;
            ++this_iter;
        }
        return true;
    }

    MonomialSubstitutionRule::const_iter_t
    MonomialSubstitutionRule::matches_anywhere(MonomialSubstitutionRule::const_iter_t iter,
                                               const MonomialSubstitutionRule::const_iter_t iter_end) const noexcept {
        while (iter != iter_end) {
            if (this->matches(iter, iter_end)) {
                return iter;
            }
            ++iter;
        }
        return iter_end;
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
        std::copy(rawRHS.cbegin(), rawRHS.cend(), std::back_inserter(output));

        // Copy remainder of input string
        hint += static_cast<ptrdiff_t>(rawLHS.size());
        std::copy(hint, input.cend(), std::back_inserter(output));

        // Post-condition sanity check
        if (output.size() != new_size) {
            throw errors::bad_hint();
        }

        return output;
    }

    size_t MonomialSubstitutionRule::all_matches(std::set<symbol_name_t>& output,
                                                 const RawSequenceBook& rsb,
                                                 const RawSequence& input) const {
        auto input_sequence = input.operators;
        assert(rsb.where(input_sequence) != nullptr);
        assert(input_sequence.size() <= rsb.longest_sequence());

        size_t match_count = 0;
        auto input_iter = input_sequence.cbegin();
        auto match_iter = this->matches_anywhere(input_iter, input_sequence.cend());
        while (match_iter != input_sequence.cend()) {
            auto altered_string = this->apply_match_with_hint(input_sequence, match_iter);
            const auto *id = rsb.where(altered_string);
            assert(id != nullptr);
            output.emplace(id->raw_id);

            // Find next match
            match_iter = this->matches_anywhere(match_iter + 1, input_sequence.cend());
            ++match_count;
        }
        return match_count;
    }

}