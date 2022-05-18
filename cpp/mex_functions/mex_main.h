/**
 * mex_main.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"
#include "mex.hpp"

#include "io_parameters.h"

#include "functions/function_list.h"
//#include "functions/function_base.h"

#include <memory>
#include <vector>
#include <iterator>

namespace NPATK::mex {

    namespace functions {
        class MexFunction;
    }

    class MexMain  {
    private:
        std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;

    public:
        explicit MexMain(std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr);

        void operator()(FlagArgumentRange outputs, FlagArgumentRange inputs);

    private:
        /**
         *
         * @param inputs The input arguments. If function name is found, it is popped from front of this list.
         * @return The ID of the function implied by the input parameters.
         */
        [[nodiscard]] functions::MEXEntryPointID get_function_id(FlagArgumentRange& inputs);

        [[nodiscard]] std::unique_ptr<functions::MexFunction> get_function(functions::MEXEntryPointID func_id);

        [[nodiscard]] SortedInputs clean_inputs(const functions::MexFunction& func, FlagArgumentRange& inputs);

        void validate_outputs(const functions::MexFunction& func, const FlagArgumentRange& outputs);

    };
}