/**
 * make_hermitian.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex_function.h"

namespace NPATK::mex::functions  {

    class MakeHermitian : public NPATK::mex::functions::MexFunction {
    public:
        explicit MakeHermitian(matlab::engine::MATLABEngine& matlabEngine);

        [[nodiscard]] std::pair<bool, std::basic_string<char16_t>> validate_inputs(const SortedInputs &input) const final;

        void operator()(FlagArgumentRange output, SortedInputs&& input) final;


    };
}
