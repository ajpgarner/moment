/**
 * imported_matrix_system.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "../../mex_function.h"

#include "integer_types.h"

namespace Moment::mex::functions {

    class ImportedMatrixSystemParams : public SortedInputs {
    public:
        bool purely_real = false;

        explicit ImportedMatrixSystemParams(SortedInputs &&rawInput);

    };

    class ImportedMatrixSystem
        : public ParameterizedMexFunction<ImportedMatrixSystemParams, MEXEntryPointID::ImportedMatrixSystem> {
    public:
        explicit ImportedMatrixSystem(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, ImportedMatrixSystemParams &input) override;
    };
}