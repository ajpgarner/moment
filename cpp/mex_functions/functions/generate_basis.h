/**
 * generate_basis.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"


namespace NPATK::mex::functions {

    struct GenerateBasisParams : public SortedInputs {
    public:
        /** True, if output should be a sparse matrix */
        bool sparse_output = false;
    public:
        explicit GenerateBasisParams(SortedInputs&& structuredInputs);
    };

    class GenerateBasis : public MexFunction {
    public:
        explicit GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

    };

}