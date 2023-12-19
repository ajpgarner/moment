/**
 * transform_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "transform_matrix.h"

#include "scenarios/derived/derived_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "storage_manager.h"

namespace Moment::mex::functions {
    TransformMatrixParams::TransformMatrixParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)}, target_system_key{matlabEngine} {
        // Get matrix system reference
        this->target_system_key.parse_input(this->inputs[0]);

        // Get matrix ID
        this->matrix_id = read_positive_integer<uint64_t>(matlabEngine, "Matrix index", this->inputs[1], 0);
    }


    TransformMatrix::TransformMatrix(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 1;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void TransformMatrix::operator()(IOArgumentRange output, TransformMatrixParams& input) {

        // First, get derived system
        std::shared_ptr<MatrixSystem> matrixSystemPtr = input.target_system_key(this->storageManager);
        assert(matrixSystemPtr); // ^-- should throw if not found

        auto& target_system = [&]() -> Derived::DerivedMatrixSystem& {
            auto* dms_ptr = dynamic_cast<Derived::DerivedMatrixSystem*>(matrixSystemPtr.get());
            if (dms_ptr) {
                return *dms_ptr;
            }
            std::stringstream errSS;
            errSS << "MatrixSystem with reference 0x" << std::hex << input.target_system_key << std::dec
                  << " was not a derived matrix system.";
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }();

        // Now, get source system, and check matrix is okay
        const auto& base_system = target_system.base_system();
        auto read_lock_base = base_system.get_read_lock();
        if (input.matrix_id >= base_system.size()) {
            std::stringstream errSS;
            errSS << "No matrix with index " << input.matrix_id << " was found in the source system.";
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        // TODO See if matrix already exists

        // Otherwise, get write lock onto derived system and trigger its creation
        auto write_lock = target_system.get_write_lock();




        throw_error(this->matlabEngine, errors::internal_error, "TransformMatrix::operator() not implemented.");
    }
}