/**
 * new_imported_matrix_system.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "new_imported_matrix_system.h"

#include "storage_manager.h"

#include "scenarios/imported/imported_matrix_system.h"

namespace Moment::mex::functions {


    NewImportedMatrixSystem::NewImportedMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                     StorageManager &storage)
         : Moment::mex::functions::MexFunction(matlabEngine, storage,
                                               MEXEntryPointID::NewImportedMatrixSystem, u"new_imported_matrix_system")
    {
        this->min_inputs = this->max_inputs = 0;
        this->min_outputs = this->max_outputs = 1;
    }

    void NewImportedMatrixSystem::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) {
        matlab::data::ArrayFactory factory;

        // Make new empty system
        std::unique_ptr<MatrixSystem> matrixSystemPtr = std::make_unique<Imported::ImportedMatrixSystem>();

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        output[0] = factory.createScalar<uint64_t>(storage_id);

    }
}