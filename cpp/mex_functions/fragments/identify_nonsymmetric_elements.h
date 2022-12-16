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
     * Read through matlab numerical matrix, and identify pairs of elements that are not symmetric.
     * @param engine Reference to matlab engine
     * @param data The data array
     * @return A SymbolSet of elements in the matrix, with raw inferred equalities.
     */
    SymbolSet identify_nonsymmetric_elements(matlab::engine::MATLABEngine &engine,
                                             const matlab::data::Array &data);


    /**
     * Check if symbolic matrix is symmetric.
     * @param engine Reference to matlab engine
     * @param data The data array
     * @return True, if the matrix is symmetric.
     */
    bool is_symmetric(matlab::engine::MATLABEngine &engine, const matlab::data::Array &data);
}
