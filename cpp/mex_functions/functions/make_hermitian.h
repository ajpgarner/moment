/**
 * make_hermitian.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace Moment::mex::functions  {


class MakeHermitian : public Moment::mex::functions::MexFunction {
    public:
        explicit MakeHermitian(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };
}
