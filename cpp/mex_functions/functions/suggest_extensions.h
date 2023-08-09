/**
 * suggest_factors.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../mtk_function.h"

namespace Moment::mex::functions  {
    struct SuggestExtensionsParams : public SortedInputs {
    public:
        /** The reference to the matrix system. */
        uint64_t matrix_system_key = 0;

        /** The reference to the matrix within the system. */
        uint64_t matrix_index = 0;

    public:
        explicit SuggestExtensionsParams(SortedInputs&& inputs);

    };



    class SuggestExtensions
            : public ParameterizedMTKFunction<SuggestExtensionsParams, MTKEntryPointID::SuggestExtensions> {
    public:
        explicit SuggestExtensions(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, SuggestExtensionsParams &input) override;

        void extra_input_checks(SuggestExtensionsParams &input) const override;
    };


}