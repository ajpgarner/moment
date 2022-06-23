/**
 * enumerate_upper_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/index_matrix_properties.h"

namespace NPATK {
    class MomentMatrix;
}

namespace NPATK::mex {
    IndexMatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                            const matlab::data::Array& matrix,
                                            IndexMatrixProperties::MatrixType basis_type);


}
