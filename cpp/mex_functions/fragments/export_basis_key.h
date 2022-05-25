/**
 * export_basis_key.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray/TypedArray.hpp"
#include "index_matrix_properties.h"
#include "mex.hpp"

namespace NPATK::mex {
    [[nodiscard]] matlab::data::TypedArray<int32_t> export_basis_key(matlab::engine::MATLABEngine& engine,
                                                               const IndexMatrixProperties& imp);

}