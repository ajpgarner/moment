/**
 * rules.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../mex_function.h"

namespace Moment::mex::functions  {

    struct RulesParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

    public:
        explicit RulesParams(SortedInputs&& inputs);
    };

    class Rules : public ParameterizedMexFunction<RulesParams, MEXEntryPointID::Rules> {
    public:
        explicit Rules(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, RulesParams &input) override;

        void extra_input_checks(RulesParams &input) const override;

    };

}
