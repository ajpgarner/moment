/**
 * add_symmetry.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex_function.h"

#include <limits>

namespace Moment::mex::functions {

    class NewSymmetrizedMatrixSystemParams : public SortedInputs {
    public:
        NewSymmetrizedMatrixSystemParams(SortedInputs&& raw_inputs);

        uint64_t matrix_system_key = 0;

        size_t max_subgroup = 0;

        size_t max_word_length = 0;
    };

    class NewSymmetrizedMatrixSystem
            : public ParameterizedMexFunction<NewSymmetrizedMatrixSystemParams,
                                              MEXEntryPointID::NewSymmetrizedMatrixSystem> {
    public:
        explicit NewSymmetrizedMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void extra_input_checks(NewSymmetrizedMatrixSystemParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, NewSymmetrizedMatrixSystemParams &input) override;

    };
}