/**
 * release.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "release.h"

#include "utilities/reporting.h"

namespace NPATK::mex::functions {

    ReleaseParams::ReleaseParams(matlab::engine::MATLABEngine &matlab, const StorageManager& storage,
                                 SortedInputs &&raw_inputs) : SortedInputs(std::move(raw_inputs)) {

        // Attempt to read MomentMatrix delete request...
        if (this->params.contains(u"moment_matrix")) {
            this->type = StorableType::MomentMatrix;
            auto mmIter = this->find_or_throw(u"moment_matrix");
            this->key = read_positive_integer(matlab, "Parameter 'moment_matrix'", mmIter, 0);
            if (!storage.MomentMatrices.check_signature(this->key)) {
                throw errors::BadInput(errors::bad_param, "Object key is not to object of requested type.");
            }
            return;
        }

        throw errors::BadInput(errors::too_few_inputs, "Type of object to be deleted must be supplied.");
    }

    Release::Release(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : MexFunction(matlabEngine, storage, MEXEntryPointID::Release, u"release") {
            this->max_outputs = 1;
            this->max_inputs = 0;
            this->min_inputs = 0;

            // Debug for mutual exclusion
            this->param_names.emplace(u"moment_matrix");
        }


    void Release::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        const auto& input = dynamic_cast<const ReleaseParams&>(*inputPtr);
        size_t remainder = 0;
        switch (input.type) {
            case ReleaseParams::StorableType::MomentMatrix:
                this->storageManager.MomentMatrices.release(input.key);
                remainder = this->storageManager.MomentMatrices.count();
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