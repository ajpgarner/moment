/**
 * moment_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operator_matrix.h"
#include "mex_function.h"

namespace NPATK {
    class Context;
}

namespace NPATK::mex::functions  {

    struct MomentMatrixParams : public OperatorMatrixParams {
    public:
        unsigned long hierarchy_level = 0;
    public:
        explicit MomentMatrixParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs)
            : OperatorMatrixParams(matlabEngine, std::move(inputs)) { }

    protected:
        void extra_parse_params(matlab::engine::MATLABEngine& matlabEngine) final;

        void extra_parse_inputs(matlab::engine::MATLABEngine& matlabEngine) final;

        [[nodiscard]] bool any_param_set() const final;

        [[nodiscard]] size_t inputs_required() const noexcept final { return 2; }

        [[nodiscard]] std::string input_format() const final { return "[matrix system ID, level]"; }
    };

    class MomentMatrix : public NPATK::mex::functions::OperatorMatrix {
    public:
        MomentMatrix(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    protected:
        std::pair<size_t, const NPATK::OperatorMatrix&>
        get_or_make_matrix(MatrixSystem& system, const OperatorMatrixParams& omp) final;
    };
}
