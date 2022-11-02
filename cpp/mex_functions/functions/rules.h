/**
 * rules.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK::mex::functions  {

    struct RulesParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

    public:
        explicit RulesParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs&& inputs);
    };

    class Rules : public NPATK::mex::functions::MexFunction {
    public:
        explicit Rules(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;

    };

}
