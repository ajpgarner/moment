/**
 * observable.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "observable.h"
#include "utilities/ipow.h"

#include <cmath>

namespace NPATK {
    size_t Observable::count_copies(size_t inflation_level) const {
        return ipow(inflation_level, this->source_count);
    }

    size_t Observable::count_operators(size_t inflation_level) const {
        return (this->outcomes-1) * this->count_copies(inflation_level);
    }

    std::vector<oper_name_t> Observable::unflatten_index(const size_t inflation_level, oper_name_t index) const {
        std::vector<oper_name_t> output(this->source_count, 0);

        // Just return 0s if no inflation
        if (inflation_level<=0) {
            return output;
        }

        const auto signed_inflation_level = static_cast<oper_name_t>(inflation_level);

        for (size_t i = 0; i < this->source_count; ++i) {
            const oper_name_t next_index = index / signed_inflation_level;
            const oper_name_t remainder = index % signed_inflation_level;
            output[this->source_count-1-i] = remainder;
            index = next_index;
        }

        return output;
    }
}
