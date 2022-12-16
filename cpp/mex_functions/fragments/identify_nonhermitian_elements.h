/**
 * identify_nonhermitian_elements.h
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
      * Read through matlab matrix, and identify pairs of elements that are not Hermitian.
      * @param engine Reference to matlab engine.
      * @param data The data array.
      * @return A SymbolSet of elements in the matrix, with raw inferred equalities.
      */
    SymbolSet identify_nonhermitian_elements(matlab::engine::MATLABEngine &engine,
                                             const matlab::data::Array &data);

     /**
      * Check if symbolic matrix is Hermitian.
      * @param engine Reference to matlab engine.
      * @param data The data array.
      * @return True, if the matrix is Hermitian.
      */
    bool is_hermitian(matlab::engine::MATLABEngine &engine, const matlab::data::Array &data);
}
