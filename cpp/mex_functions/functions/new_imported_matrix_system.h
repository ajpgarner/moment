/**
 * new_imported_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../mex_function.h"

#include "integer_types.h"

namespace Moment::mex::functions {

    class NewImportedMatrixSystemParams : public SortedInputs {
    public:
        bool purely_real = false;

        explicit NewImportedMatrixSystemParams(SortedInputs &&rawInput);

    };

    class NewImportedMatrixSystem
        : public ParameterizedMexFunction<NewImportedMatrixSystemParams, MEXEntryPointID::NewImportedMatrixSystem> {
    public:
        explicit NewImportedMatrixSystem(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, NewImportedMatrixSystemParams &input) override;
    };
}