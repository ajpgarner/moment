/**
 * matrix_system.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "matlab_class.h"

namespace NPATK::mex::classes {
    class MatrixSystem : public MATLABClass {
    private:
        uint64_t reference_key = 0;
    public:
        MatrixSystem(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput);

        [[nodiscard]] constexpr uint64_t Key() const noexcept { return this->reference_key; }
    };
}