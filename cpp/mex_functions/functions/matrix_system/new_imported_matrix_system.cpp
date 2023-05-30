/**
 * new_imported_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "new_imported_matrix_system.h"

#include "storage_manager.h"

#include "scenarios/imported/imported_matrix_system.h"

namespace Moment::mex::functions {

    NewImportedMatrixSystemParams::NewImportedMatrixSystemParams(SortedInputs &&rawInput)
         : SortedInputs(std::move(rawInput)) {
        if (this->flags.contains(u"real")) {
            this->purely_real = true;
        } else if (this->flags.contains(u"complex")) {
            this->purely_real = false;
        }

    }

    NewImportedMatrixSystem::NewImportedMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                     StorageManager &storage)
         : ParameterizedMexFunction{matlabEngine, storage}
    {
        this->flag_names.insert(u"real");
        this->flag_names.insert(u"complex");
        this->mutex_params.add_mutex(u"real", u"complex");
        this->min_inputs = this->max_inputs = 0;
        this->min_outputs = this->max_outputs = 1;
    }

    void NewImportedMatrixSystem::operator()(IOArgumentRange output, NewImportedMatrixSystemParams &input) {
        // Make new empty system
        std::unique_ptr<MatrixSystem> matrixSystemPtr
            = std::make_unique<Imported::ImportedMatrixSystem>(input.purely_real);

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);
    }

}