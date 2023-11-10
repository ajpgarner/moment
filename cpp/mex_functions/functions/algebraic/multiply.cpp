/**
 * multiply.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "multiply.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"
#include "utilities/read_as_vector.h"

#include "storage_manager.h"

#include "scenarios/context.h"

namespace Moment::mex::functions {

    MultiplyParams::MultiplyParams(SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get matrix system reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

    }

    void Multiply::extra_input_checks(MultiplyParams& input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }

    Multiply::Multiply(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void Multiply::operator()(IOArgumentRange output, MultiplyParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error &poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;

        throw_error(this->matlabEngine, errors::internal_error, "Multiply::operator() not implemented.");

    }

    void Multiply::validate_op_seq(const Context& context, std::span<const oper_name_t> operator_string, size_t index) const {
        size_t idx = 1; // MATLAB 1-indexing
        for (auto op_num : operator_string) {
            if ((op_num < 0) || (op_num >= context.size())) {
                std::stringstream errSS;
                errSS << "Operator " << (op_num + 1) << " at index " << idx;
                if (index > 0) {
                    errSS << " of entry " << index;
                }
                errSS << " is out of range.";
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
            ++idx;
        }

    }

}
