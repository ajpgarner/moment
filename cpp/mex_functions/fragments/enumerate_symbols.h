/**
 * enumerate_upper_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "matrix/operator_matrix.h"
#include "matrix/matrix_properties.h"

#include <map>

namespace Moment {
    class MomentMatrix;
}

namespace Moment::mex {
    /**
     * Scan supplied matrix for symbols, ascertaining which must be Hermitian, and which are unconstrained.
     * @param engine The MATLAB engine.
     * @param matrix MATLAB array of symbols.
     * @param basis_type Whether the matrix should be read as symmetric or as Hermitian.
     * @return A SymbolMatrixProperties class listing the found symbols.
     */
    MatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                                            const matlab::data::Array& matrix,
                                                            MatrixType basis_type);

}
