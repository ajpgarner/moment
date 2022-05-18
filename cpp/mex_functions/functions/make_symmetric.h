/**
 * make_symmetric.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "function_base.h"

namespace NPATK::mex::functions {
    class MakeSymmetric : public MexFunction {
        public:
            explicit MakeSymmetric(matlab::engine::MATLABEngine& matlabEngine);

            void operator()(FlagArgumentRange output, SortedInputs&& input) final;

    };
}