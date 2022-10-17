/**
 * suffix_prefix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <algorithm>
#include <vector>

namespace NPATK {

    template<class type_t>
    size_t suffix_prefix(const std::vector<type_t>& lhs, const std::vector<type_t>& rhs) noexcept {

        size_t match_count = std::min(lhs.size(), rhs.size());
        // Early exit, if one string is null.
        if (0 == match_count) {
            return 0;
        }

        auto lhs_tail_iter = lhs.cbegin() + static_cast<ptrdiff_t>(lhs.size() - match_count);
        auto rhs_head_iter = rhs.cbegin();

        while (match_count > 0) {
            auto [found_mismatch_lhs, found_mismatch_rhs] = std::mismatch(lhs_tail_iter, lhs.cend(),
                                                                          rhs_head_iter, rhs.cend());
            if (found_mismatch_lhs == lhs.cend()) {
                return match_count;
            }

            --match_count;
            ++lhs_tail_iter;
        }

        return 0;
    }
}