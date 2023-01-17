/**
 * import_matrix.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "import_matrix.h"

#include "storage_manager.h"

#include "matrix/symbolic_matrix.h"

#include "scenarios/imported/imported_matrix_system.h"

#include "fragments/read_raw_symbol_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {
    ImportMatrixParams::ImportMatrixParams(matlab::engine::MATLABEngine &matlabEngine,
                                           Moment::mex::SortedInputs &&rawInputs)
       : SortedInputs(std::move(rawInputs)), inputMatrix{this->inputs[1]} {
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Verify form of second argument...!
        auto dimensions = this->inputMatrix.getDimensions();
        if ((dimensions.size() != 2) || (dimensions[0] != dimensions[1])) {
            throw errors::BadInput{errors::bad_param, "Input must be square matrix."};
        }

    }

    ImportMatrix::ImportMatrix(matlab::engine::MATLABEngine &matlabEngine, Moment::mex::StorageManager &storage)
        : Moment::mex::functions::MexFunction(matlabEngine, storage, MEXEntryPointID::ImportMatrix, u"import_matrix") {
        this->min_inputs = this->max_inputs = 2;
        this->min_outputs = this->max_outputs = 1;
    }

    std::unique_ptr<SortedInputs> ImportMatrix::transform_inputs(std::unique_ptr<SortedInputs> input) const {
        auto processed = std::make_unique<ImportMatrixParams>(this->matlabEngine, std::move(*input));

        if (!this->storageManager.MatrixSystems.check_signature(processed->matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }

        return processed;
    }

    void ImportMatrix::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        // Get input
        assert(inputPtr);
        auto& input = dynamic_cast<ImportMatrixParams&>(*inputPtr);

        // Attempt to get matrix system, and cast to ImportedMatrixSystem
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch(const persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        MatrixSystem& matrixSystem = *matrixSystemPtr;
        auto* imsPtr = dynamic_cast<Imported::ImportedMatrixSystem*>(&matrixSystem);
        if (nullptr == imsPtr) {
            std::stringstream errSS;
            errSS << "MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec
                  << " was not a valid ImportedMatrixSystem.";
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        Imported::ImportedMatrixSystem& ims = *imsPtr;

        // Read input to raw form
        auto raw_sym_mat = read_raw_symbol_matrix(this->matlabEngine, input.inputMatrix);
        assert(raw_sym_mat);

        // Lock for import
        auto write_lock = ims.get_write_lock();

        // Do import
        size_t matrix_index = ims.import_matrix(std::move(raw_sym_mat));

        // Output created matrix ID
        matlab::data::ArrayFactory factory;
        if (output.size() > 0) {
            output[0] = factory.createScalar(matrix_index);
        }
    }
}