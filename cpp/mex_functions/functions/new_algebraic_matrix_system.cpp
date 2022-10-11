/**
 * new_algebraic_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "new_algebraic_matrix_system.h"
#include "utilities/reporting.h"

namespace NPATK::mex::functions {


    NewAlgebraicMatrixSystem::NewAlgebraicMatrixSystem(matlab::engine::MATLABEngine &matlabEngine,
                                                       StorageManager &storage)
           : MexFunction(matlabEngine, storage,
                         MEXEntryPointID::NewAlgebraicMatrixSystem,
                         u"new_algebraic_matrix_system") {

    }

    void NewAlgebraicMatrixSystem::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) {
        throw_error(this->matlabEngine, errors::internal_error, "Not implemented.");
    }

    std::unique_ptr<SortedInputs> NewAlgebraicMatrixSystem::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        return input;
    }
}