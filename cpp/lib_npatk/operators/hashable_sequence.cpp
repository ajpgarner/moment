/**
 * hashable_sequence.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "hashable_sequence.h"

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


}