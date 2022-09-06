/**
 * moment_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "matlab_class.h"

namespace NPATK::mex {
    namespace classes {
        class MomentMatrix : public MATLABClass {
        private:
            uint64_t reference_key = 0;
            uint64_t level = 0;
        public:
            MomentMatrix(matlab::engine::MATLABEngine &engine, matlab::data::Array rawInput);

            [[nodiscard]] constexpr uint64_t SystemKey() const noexcept { return this->reference_key; }

            [[nodiscard]] constexpr uint64_t Level() const noexcept { return this->level; }
        };
    }

    /**
    * Check if supplied array object matches a valid specification of (matlab) 'MomentMatrix' class.
    * @param engine Handle to MATLAB engine.
    * @param raw_data The raw object to verify.
    * @return Pair: first: true ptr to Scenario object, if valid; second: why verification failed, set when first is null.
    */
    [[nodiscard]] std::pair<std::unique_ptr<classes::MomentMatrix>, std::optional<std::string>>
    read_as_moment_matrix(matlab::engine::MATLABEngine& engine, matlab::data::Array raw_data);
}