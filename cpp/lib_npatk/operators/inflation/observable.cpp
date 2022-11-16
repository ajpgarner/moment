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
        return ipow(inflation_level, this->sources.size());
    }

    size_t Observable::count_operators(size_t inflation_level) const {
        return (this->outcomes-1) * this->count_copies(inflation_level);
    }
}
