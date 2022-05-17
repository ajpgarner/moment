/**
 * function_base.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>
#include <map>

#include "function_list.h"
#include "../wrapped_arg_range.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex::functions {

    class MexFunction {
    protected:
        matlab::engine::MATLABEngine& matlabEngine;

    public:
        const MEXEntryPointID function_id;

        MexFunction(matlab::engine::MATLABEngine& engine, MEXEntryPointID id)
            : matlabEngine(engine), function_id{id} { }

        virtual ~MexFunction() = default;

        virtual void operator()(WrappedArgRange output, WrappedArgRange input) = 0;

    };
}