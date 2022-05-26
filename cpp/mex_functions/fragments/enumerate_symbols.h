/**
 * enumerate_upper_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "index_matrix_properties.h"

namespace NPATK::mex {
    IndexMatrixProperties enumerate_upper_symbols(matlab::engine::MATLABEngine& engine,
                                                  const matlab::data::Array& matrix,
                                                  IndexMatrixProperties::MatrixType basis_type,
                                                  bool debug_output = false);

}
