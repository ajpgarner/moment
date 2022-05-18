/**
 * version.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "function_base.h"

namespace NPATK::mex::functions {

    class Version : public MexFunction {

    public:
        explicit Version(matlab::engine::MATLABEngine& matlabEngine);



        void operator()(FlagArgumentRange output, SortedInputs&& input) final;

    };

}