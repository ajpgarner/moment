/**
 * echo.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../../mtk_function.h"

namespace Moment::mex::functions  {

    struct EchoMatrixParams : public SortedInputs {
    public:
        explicit EchoMatrixParams(SortedInputs&& inputs);

        enum OutputMode {
            Dense,
            Sparse
        } output_mode = OutputMode::Dense;

        enum MatrixMode {
            Real,
            Complex
        } matrix_mode = MatrixMode::Real;
    };

    class EchoMatrix : public ParameterizedMTKFunction<EchoMatrixParams, MTKEntryPointID::EchoMatrix> {
    public:
        explicit EchoMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, EchoMatrixParams &input) override;

    };

}
