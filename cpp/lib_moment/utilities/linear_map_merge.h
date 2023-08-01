/**
 * linear_map_merge.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <concepts>
#include <map>

namespace Moment {

    /**
     * Adds elements from RHS map to LHS map in linear time.
     */
    template<std::integral index_t, typename value_t>
    void linear_map_merge(std::map<index_t, value_t>& lhs, std::map<index_t, value_t>&& rhs) {

        auto this_iter = lhs.begin();
        auto other_iter = rhs.begin();

        while (other_iter != rhs.end()) {

            // Copy all remaining?
            if (this_iter == lhs.end()) {
                lhs.insert(this_iter, std::make_pair(other_iter->first, std::move(other_iter->second)));
                ++other_iter;
                continue;
            }

            const size_t try_hash = other_iter->first;

            // Can directly add
            if (this_iter->first < try_hash) {
                ++this_iter;
                continue;
            } else if (this_iter->first > try_hash) {
                // Insert just before this element
                this_iter = lhs.insert(this_iter, std::make_pair(try_hash, std::move(other_iter->second)));
                ++other_iter;
                continue;
            } else {
                // Same element, can skip
                ++this_iter;
                ++other_iter;
            }
        }
    }
}