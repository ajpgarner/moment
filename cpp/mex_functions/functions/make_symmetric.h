/**
 * make_symmetric.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK::mex::functions {

    struct MakeSymmetricParams : public SortedInputs {
    public:
        /** True, if output should be a sparse matrix */
        bool sparse_output = false;
    public:
        explicit MakeSymmetricParams(SortedInputs&& structuredInputs);
    };

    class MakeSymmetric : public MexFunction {
    public:
        explicit MakeSymmetric(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };
}