/**
 * echo.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../mex_function.h"

namespace Moment::mex::functions  {

    struct EchoParams : public SortedInputs {
    public:
        explicit  EchoParams(SortedInputs&& inputs);

        enum OutputMode {
            Dense,
            Sparse
        } output_mode = OutputMode::Dense;

        enum MatrixMode {
            Real,
            Complex
        } matrix_mode = MatrixMode::Real;
    };

    class Echo : public ParameterizedMexFunction<EchoParams, MEXEntryPointID::Echo> {
    public:
        explicit Echo(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, EchoParams &input) override;

    };

}
