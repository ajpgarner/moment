/**
 * export_basis_key.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/matrix/symbol_matrix_properties.h"

#include "MatlabDataArray/TypedArray.hpp"

#include "mex.hpp"

namespace NPATK::mex {
    [[nodiscard]] matlab::data::TypedArray<int32_t> export_basis_key(matlab::engine::MATLABEngine& engine,
                                                               const SymbolMatrixProperties& imp);

}