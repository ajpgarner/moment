/**
 * release.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "release.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {

    ReleaseParams::ReleaseParams(SortedInputs &&raw_inputs)
        : SortedInputs(std::move(raw_inputs)) {

        // Attempt to read MomentMatrix delete request...
        if (this->params.contains(u"matrix_system")) {
            this->type = StorableType::MatrixSystem;
            auto mmIter = this->find_or_throw(u"matrix_system");
            this->key = read_positive_integer<uint64_t>(this->matlabEngine, "Parameter 'matrix_system'", mmIter, 0);
            return;
        }

        throw errors::BadInput(errors::too_few_inputs, "Type of object to be deleted must be supplied.");
    }

    Release::Release(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->max_outputs = 1;
        this->max_inputs = 0;
        this->min_inputs = 0;

        this->param_names.emplace(u"matrix_system");
    }

    void Release::extra_input_checks(ReleaseParams &input) const {
        if (input.type == ReleaseParams::StorableType::MatrixSystem) {
            if (!this->storageManager.MatrixSystems.check_signature(input.key)) {
                throw errors::BadInput(errors::bad_param, "Object key is not to object of requested type.");
            }
        }
    }

    void Release::operator()(IOArgumentRange output, ReleaseParams &input) {
        size_t remainder = 0;
        switch (input.type) {
            case ReleaseParams::StorableType::MatrixSystem:
                try {
                    this->storageManager.MatrixSystems.release(input.key);
                } catch (const std::exception& e) {
                    throw_error(this->matlabEngine, errors::internal_error, e.what());
                }
                remainder = this->storageManager.MatrixSystems.size();
                break;
            default:
            case ReleaseParams::StorableType::Unknown:
                throw_error(this->matlabEngine, errors::internal_error, "Not implemented.");
        }

        // Return number of objects left in storage...
        if (output.size() >= 1) {
            matlab::data::ArrayFactory arrayFactory;
            output[0] = arrayFactory.createScalar<uint64_t>(remainder);
        }
    }
}