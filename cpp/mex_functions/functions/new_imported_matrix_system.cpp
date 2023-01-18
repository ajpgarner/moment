/**
 * new_imported_matrix_system.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "new_imported_matrix_system.h"

#include "storage_manager.h"

#include "scenarios/imported/imported_matrix_system.h"

namespace Moment::mex::functions {

    NewImportedMatrixSystemParams::NewImportedMatrixSystemParams(matlab::engine::MATLABEngine &matlabEngine,
                                                                 SortedInputs &&rawInput)
         : SortedInputs(std::move(rawInput)) {
        if (this->flags.contains(u"real")) {
            this->purely_real = true;
        } else {
            this->purely_real = false;
        }
    }

    NewImportedMatrixSystem::NewImportedMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                     StorageManager &storage)
         : Moment::mex::functions::MexFunction(matlabEngine, storage,
                                               MEXEntryPointID::NewImportedMatrixSystem, u"new_imported_matrix_system")
    {
        this->flag_names.insert(u"real");
        this->min_inputs = this->max_inputs = 0;
        this->min_outputs = this->max_outputs = 1;
    }

    void NewImportedMatrixSystem::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        matlab::data::ArrayFactory factory;

        auto& input = dynamic_cast<NewImportedMatrixSystemParams&>(*inputPtr);

        // Make new empty system
        std::unique_ptr<MatrixSystem> matrixSystemPtr
            = std::make_unique<Imported::ImportedMatrixSystem>(input.purely_real);

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        output[0] = factory.createScalar<uint64_t>(storage_id);

    }

    std::unique_ptr<SortedInputs> NewImportedMatrixSystem::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        return std::make_unique<NewImportedMatrixSystemParams>(this->matlabEngine, std::move(*input));
    }

}