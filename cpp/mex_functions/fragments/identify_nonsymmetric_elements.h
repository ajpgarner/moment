/**
 * identify_nonsymmetric_elements.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "symbolic/symbol_set.h"

namespace matlab {
    namespace engine {
        class MATLABEngine;
    }
    namespace data {
        class Array;
    }
}

namespace Moment::mex {
    /**
     * Check if symbolic matrix is symmetric.
     * @param engine Reference to matlab engine
     * @param data The data array
     * @return True, if the matrix is symmetric.
     */
    bool is_symmetric(matlab::engine::MATLABEngine &engine, const matlab::data::Array &data);
}
