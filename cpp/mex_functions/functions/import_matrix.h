/**
 * import_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mex_function.h"

#include "integer_types.h"

#include <string>

namespace Moment::mex::functions  {

    struct ImportMatrixParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;

        bool matrix_is_complex = true;

        bool matrix_is_hermitian = false;

        matlab::data::Array inputMatrix;

    public:
        explicit ImportMatrixParams(SortedInputs&& inputs);

    };

    class ImportMatrix : public ParameterizedMexFunction<ImportMatrixParams, MEXEntryPointID::ImportMatrix> {
    public:
        ImportMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ImportMatrixParams &input) override;

    protected:
        void extra_input_checks(ImportMatrixParams &input) const override;
    };
}