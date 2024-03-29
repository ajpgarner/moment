/**
 * import_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mtk_function.h"

#include "import/matrix_system_id.h"
#include "integer_types.h"

#include <string>

namespace Moment::mex::functions  {

    struct ImportMatrixParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        bool matrix_is_complex = true;

        bool matrix_is_hermitian = false;

        matlab::data::Array inputMatrix;

    public:
        explicit ImportMatrixParams(SortedInputs&& inputs);

    };

    class ImportMatrix : public ParameterizedMTKFunction<ImportMatrixParams, MTKEntryPointID::ImportMatrix> {
    public:
        ImportMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ImportMatrixParams &input) override;
    };
}