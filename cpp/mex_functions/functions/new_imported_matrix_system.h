/**
 * new_imported_matrix_system.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once
#include "mex_function.h"

#include "integer_types.h"

namespace Moment::mex::functions {


    class NewImportedMatrixSystem : public Moment::mex::functions::MexFunction {
    public:
        explicit NewImportedMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) final;

    };
}