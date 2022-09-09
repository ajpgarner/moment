/**
 * enumerate_upper_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "operators/matrix/operator_matrix.h"
#include "operators/matrix/symbol_matrix_properties.h"

#include <map>

namespace NPATK {
    class MomentMatrix;
}

namespace NPATK::mex {
    /**
     * Scan supplied matrix for symbols, ascertaining which must be Hermitian, and which are unconstrained.
     * @param engine The MATLAB engine.
     * @param matrix MATLAB array of symbols.
     * @param basis_type Whether the matrix should be read as symmetric or as Hermitian.
     * @return A SymbolMatrixProperties class listing the found symbols.
     */
    SymbolMatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                                            const matlab::data::Array& matrix,
                                                            MatrixType basis_type);

}
