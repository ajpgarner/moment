/**
 * enumerate_upper_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "operators/matrix/operator_matrix.h"

namespace NPATK {
    class MomentMatrix;
}

namespace NPATK::mex {
    SymbolMatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                                            const matlab::data::Array& matrix,
                                                            MatrixType basis_type);


}
