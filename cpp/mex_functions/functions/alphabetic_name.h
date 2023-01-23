/**
 * alphabetic_name.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once


#include "../mex_function.h"

namespace Moment::mex::functions  {

    class AlphabeticNameInputs : public SortedInputs {
    public:
        bool is_upper = false;
        bool zero_index = false;

        explicit AlphabeticNameInputs(matlab::engine::MATLABEngine& matlabEngine, SortedInputs&& input);
    };

    class AlphabeticName : public ParameterizedMexFunction<AlphabeticNameInputs, MEXEntryPointID::AlphabeticName> {
    public:
        explicit AlphabeticName(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, AlphabeticNameInputs &input) override;
    };
}
