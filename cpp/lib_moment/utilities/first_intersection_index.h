/**
 * first_intersection_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <functional>
#include <optional>

namespace Moment {


    /**
     * Gets the index of the first match index that exists in the test range.
     * Both test and match ranges must be sorted according to the same predicate supplied as 'less_than'.
     * @tparam test_iter_t Iterator type for test range.
     * @tparam match_iter_t Iterator type for match range.
     * @tparam comparator_t Comparator '<', such that !(a < b) && !(b < a) implies a == b.
     * @param test_iter The start of the test range.
     * @param test_iter_end The end of the test range.
     * @param match_iter The start of the match patterns.
     * @param match_iter_end The end of the match patterns.
     * @param less_than The instance of the comparator.
     * @return
     */
    template<typename test_iter_t, typename match_iter_t,
            typename comparator_t = std::less<typename test_iter_t::value_type>>
    std::optional<size_t>
    first_intersection_index(test_iter_t test_iter, const test_iter_t test_iter_end,
                             match_iter_t match_iter, const match_iter_t match_iter_end,
                             comparator_t less_than = comparator_t{}) {
        static_assert(std::is_convertible_v<typename test_iter_t::value_type, typename match_iter_t::value_type>);

        size_t match_index = 0;
        while ((test_iter != test_iter_end) && (match_iter != match_iter_end)) {
            if (less_than(*test_iter, *match_iter)) { // test iter lags
                ++test_iter;
            } else if (less_than(*match_iter, *test_iter)) { // match iter lags
                ++match_iter;
                ++match_index;
            } else { // match
                return match_index;
            }
        }
        return std::nullopt;
    }



}