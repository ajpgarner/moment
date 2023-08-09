/**
 * alphabetic_name.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once


#include "mtk_function.h"

namespace Moment::mex::functions  {

    class AlphabeticNameParams : public SortedInputs {
    public:
        bool is_upper = false;
        bool zero_index = false;

        explicit AlphabeticNameParams(SortedInputs&& input);
    };

    class AlphabeticName : public ParameterizedMTKFunction<AlphabeticNameParams, MTKEntryPointID::AlphabeticName> {
    public:
        explicit AlphabeticName(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, AlphabeticNameParams &input) override;
    };
}
