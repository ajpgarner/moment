/**
 * import_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"
#include "operator_matrix.h"

#include "integer_types.h"

#include "matrix/matrix_type.h"

#include <string>

namespace Moment::mex::functions  {

    struct ImportMatrixParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        MatrixType input_matrix_type = MatrixType::Unknown;

        matlab::data::Array inputMatrix;

    public:
        ImportMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

    };

    class ImportMatrix : public Moment::mex::functions::MexFunction {
    public:
        ImportMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };
}