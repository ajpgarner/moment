/**
 * add_symmetry.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex_function.h"

namespace Moment::mex::functions {

    class AddSymmetryParams : public SortedInputs {
    public:
        AddSymmetryParams(SortedInputs&& raw_inputs);

        uint64_t matrix_system_key = 0;

        size_t max_subgroup = 0;
    };

    class AddSymmetry : public ParameterizedMexFunction<AddSymmetryParams, MEXEntryPointID::AddSymmetry> {
    public:
        explicit AddSymmetry(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void extra_input_checks(AddSymmetryParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, AddSymmetryParams &input) override;

    };
}