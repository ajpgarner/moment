/**
 * version.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../../version.h"
#include "../../mtk_function.h"

namespace Moment::mex::functions {

    class Version : public MTKFunction {
    public:
        explicit Version(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final {
            return std::move(input);
        }

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;
    };

}