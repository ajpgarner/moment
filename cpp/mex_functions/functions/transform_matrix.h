/**
 * transform_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "mtk_function.h"
#include "import/matrix_system_id.h"

namespace Moment::mex::functions {
    struct TransformMatrixParams : public SortedInputs {
    public:
        /** The key to the symmetrized matrix system. */
        MatrixSystemId target_system_key;

        /** The ID of the matrix to transform. */
        uint64_t matrix_id;

    public:
        explicit TransformMatrixParams(SortedInputs&& structuredInputs);

    };

    class TransformMatrix : public ParameterizedMTKFunction<TransformMatrixParams, MTKEntryPointID::TransformMatrix> {
    public:
        explicit TransformMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage);

    protected:
        void operator()(IOArgumentRange output, TransformMatrixParams& input) override;

    };
}