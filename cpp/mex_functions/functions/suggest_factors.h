/**
 * suggest_factors.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "../mex_function.h"

namespace Moment::mex::functions  {
    struct SuggestFactorsParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The reference to the matrix within the system. */
        uint64_t matrix_index = 0;

    public:
        explicit SuggestFactorsParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);

    };



    class SuggestFactors : public ParameterizedMexFunction<SuggestFactorsParams, MEXEntryPointID::SuggestFactors> {
    public:
        explicit SuggestFactors(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, SuggestFactorsParams &input) override;

        void extra_input_checks(SuggestFactorsParams &input) const override;
    };


}