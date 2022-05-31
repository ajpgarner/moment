/**
 * mex_main.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"
#include "mex.hpp"

#include "utilities/io_parameters.h"

#include "functions/function_list.h"

#include <memory>
#include <vector>
#include <iterator>

namespace NPATK::mex {

    namespace functions {
        class MexFunction;
    }

    namespace errors {
        /** Error code: thrown when function is not recognised */
        constexpr char bad_function[] = "bad_function";

        /** Error code: thrown when known parameter encountered, but input following was bad. */
        constexpr char bad_param[] = "bad_param";

        /** Error code: thrown when two or more mutually exclusive flags/parameters are provided. */
        constexpr char mutex_param[] = "mutex_param";

        /** Error code: thrown when inputs are missing */
        constexpr char too_few_inputs[] = "too_few_inputs";

        /** Error code: thrown when there are too many inputs */
        constexpr char too_many_inputs[] = "too_many_inputs";

        /** Error code: thrown when outputs are missing */
        constexpr char too_few_outputs[] = "too_few_outputs";

        /** Error code: thrown when there are too many outputs */
        constexpr char too_many_outputs[] = "too_many_outputs";
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

        [[nodiscard]] SortedInputs clean_inputs(const functions::MexFunction& func, FlagArgumentRange& inputs);

        void validate_inputs(const functions::MexFunction& func, const SortedInputs& inputs);

        void validate_outputs(const functions::MexFunction& func, const FlagArgumentRange& outputs);

    };
}