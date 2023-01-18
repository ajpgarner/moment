/**
 * new_imported_matrix_system.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"

#include "integer_types.h"

namespace Moment::mex::functions {

    class NewImportedMatrixSystemParams : public SortedInputs {
    public:
        bool purely_real = false;

        NewImportedMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine,
                                      SortedInputs &&rawInput);

    };

    class NewImportedMatrixSystem : public Moment::mex::functions::MexFunction {
    public:
        explicit NewImportedMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

        [[nodiscard]] std::unique_ptr<SortedInputs> transform_inputs(std::unique_ptr<SortedInputs> input) const final;
    };
}