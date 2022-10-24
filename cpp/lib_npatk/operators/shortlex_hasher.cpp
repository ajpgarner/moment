/**
 * shortlex_hasher.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "shortlex_hasher.h"

#include <cmath>

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

    size_t ShortlexHasher::longest_hashable_string() const {
        if (this->radix == 1) {
            return std::numeric_limits<size_t>::max() - 1; // Hash is basically just string length for radix 1.
        }
        return static_cast<size_t>(
            static_cast<double>(std::numeric_limits<size_t>::digits) / std::log2(this->radix)
        );
    }
}