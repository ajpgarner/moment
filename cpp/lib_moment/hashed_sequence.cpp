/**
 * hashable_sequence.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "hashed_sequence.h"

#include <algorithm>
#include <iostream>

namespace Moment {
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

    std::ostream& operator<<(std::ostream& os, const HashedSequence& seq) {
        if (seq.negated()) {
            os << "-";
        }
        if (seq.empty()) {
            if (seq.zero()) {
                os << "0";
            } else {
                os << "I";
            }
        } else {
            for (const auto o : seq.operators) {
                os << "X" << o;
            }
        }
        os << " [" << seq.the_hash << "]";
        return os;
    }


}