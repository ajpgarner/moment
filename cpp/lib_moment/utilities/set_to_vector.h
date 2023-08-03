/**
 * set_to_vector.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <algorithm>
#include <iterator>
#include <set>
#include <vector>

namespace Moment {
    template<typename type_t>
    std::vector<type_t> set_to_vector(const std::set<type_t>& input) {
        std::vector<type_t> output;
        output.reserve(input.size());
        std::copy(input.cbegin(), input.cend(), std::back_inserter(output));
        return output;
    }
}