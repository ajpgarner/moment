/**
 * enumerate_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "index_matrix_properties.h"

namespace NPATK::mex {
    IndexMatrixProperties enumerate_symbols(matlab::engine::MATLABEngine& engine,
                                            const matlab::data::Array& matrix,
                                            IndexMatrixProperties::BasisType basis_type,
                                            bool debug_output = false);

}
