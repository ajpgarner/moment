/**
 * symmetrized_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mtk_function.h"

#include <limits>

namespace Moment::mex::functions {

    class SymmetrizedMatrixSystemParams : public SortedInputs {
    public:
        SymmetrizedMatrixSystemParams(SortedInputs&& raw_inputs);

        uint64_t matrix_system_key = 0;

        size_t max_subgroup = 0;

        size_t max_word_length = 0;

        double zero_tolerance = -1.0; // deduce from base system.
    };

    class SymmetrizedMatrixSystem : public ParameterizedMTKFunction<SymmetrizedMatrixSystemParams,
                                                                    MTKEntryPointID::SymmetrizedMatrixSystem> {
    public:
        explicit SymmetrizedMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void extra_input_checks(SymmetrizedMatrixSystemParams &input) const override;

    protected:
        void operator()(IOArgumentRange output, SymmetrizedMatrixSystemParams &input) override;

    };
}