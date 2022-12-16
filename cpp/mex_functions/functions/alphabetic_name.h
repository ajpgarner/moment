/**
 * alphabetic_name.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once


#include "mex_function.h"

namespace Moment::mex::functions  {

    class AlphabeticNameInputs : public SortedInputs {
    public:
        bool is_upper = false;
        bool zero_index = false;

        explicit AlphabeticNameInputs(SortedInputs&& input);
    };

    class AlphabeticName : public Moment::mex::functions::MexFunction {
    public:
        explicit AlphabeticName(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;
    };
}
