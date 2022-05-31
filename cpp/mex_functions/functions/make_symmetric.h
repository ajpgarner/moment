/**
 * make_symmetric.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK::mex::functions {
    class MakeSymmetric : public MexFunction {
        public:
            explicit MakeSymmetric(matlab::engine::MATLABEngine& matlabEngine);

            void operator()(FlagArgumentRange output, SortedInputs&& input) final;

        [[nodiscard]] std::pair<bool, std::basic_string<char16_t>> validate_inputs(const SortedInputs &input) const final;

    };
}