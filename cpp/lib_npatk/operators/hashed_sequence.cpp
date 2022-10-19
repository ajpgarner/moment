/**
 * hashable_sequence.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "hashed_sequence.h"

#include <algorithm>

namespace NPATK {

    size_t ShortlexHasher::hash(const std::vector<oper_name_t>& rawOperators) const noexcept {
        size_t hash = 1;
        size_t multiplier = 1;
        const size_t multiplier_stride = 1 + radix;
        const size_t len = rawOperators.size();

        for (size_t n = 0; n < len; ++n) {
            const auto& oper = rawOperators[len-n-1];
            hash += ((static_cast<size_t>(oper) + 1) * multiplier);
            multiplier *= multiplier_stride;
        }
        return hash;
    }

    bool HashedSequence::matches(HashedSequence::const_iter_t test_iter,
                                 HashedSequence::const_iter_t test_iter_end) const noexcept {
        // No match, if not enough space left
        if (std::distance(test_iter, test_iter_end) < this->operators.size()) {
            return false;
        }

        auto [mm_this, mm_test] = std::mismatch(this->operators.cbegin(), this->operators.cend(),
                                                test_iter, test_iter_end);

        return (mm_this == this->operators.cend());
    }

    HashedSequence::const_iter_t
    HashedSequence::matches_anywhere(HashedSequence::const_iter_t iter,
                                     HashedSequence::const_iter_t iter_end) const noexcept {
        while (iter != iter_end) {
            if (this->matches(iter, iter_end)) {
                return iter;
            }
            ++iter;
        }
        return iter_end;
    }

    [[nodiscard]] ptrdiff_t HashedSequence::suffix_prefix_overlap(const HashedSequence& rhs) const noexcept {
        const auto& lhs = *this;

        ptrdiff_t match_count = static_cast<ptrdiff_t>(std::min(lhs.size(), rhs.size()));
        // Early exit, if one string is null.
        if (0 == match_count) {
            return 0;
        }

        auto lhs_tail_iter = lhs.operators.cbegin() + static_cast<ptrdiff_t>(lhs.operators.size() - match_count);
        auto rhs_head_iter = rhs.operators.cbegin();

        while (match_count > 0) {
            auto [found_mismatch_lhs, found_mismatch_rhs] = std::mismatch(lhs_tail_iter, lhs.operators.cend(),
                                                                          rhs_head_iter, rhs.operators.cend());
            if (found_mismatch_lhs == lhs.operators.cend()) {
                return match_count;
            }

            --match_count;
            ++lhs_tail_iter;
        }
        return 0;
    }


}