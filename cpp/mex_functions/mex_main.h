/**
 * mex_main.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"
#include "mex.hpp"

#include "wrapped_arg_range.h"

#include "functions/function_list.h"
#include "functions/function_base.h"

#include <memory>
#include <vector>
#include <iterator>

namespace NPATK::mex {

    class MexMain  {
    private:
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;

    public:
        explicit MexMain(std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr);

        void operator()(WrappedArgRange outputs, WrappedArgRange inputs);

    private:
        [[nodiscard]] functions::MEXEntryPointID get_function_id(WrappedArgRange inputs);
    };
}