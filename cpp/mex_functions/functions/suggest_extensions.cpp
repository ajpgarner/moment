/**
 * suggest_factors.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "suggest_extensions.h"

#include "storage_manager.h"
#include "matrix/moment_matrix.h"

#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/write_as_array.h"

namespace Moment::mex::functions  {
    namespace {
        const MonomialMatrix& getMatrixOrThrow(matlab::engine::MATLABEngine &matlabEngine,
                                               const MatrixSystem& matrixSystem, size_t index) {
            try {
                return matrixSystem[index];
            } catch (const Moment::errors::missing_component& mce) {
                throw_error(matlabEngine, errors::bad_param, mce.what());
            }
        }
    }

    SuggestExtensionsParams::SuggestExtensionsParams(SortedInputs &&rawInputs)
        : SortedInputs(std::move(rawInputs)) {
        this->matrix_system_key = read_as_scalar<uint64_t>(matlabEngine, this->inputs[0]);
        this->matrix_index = read_as_scalar<uint64_t>(matlabEngine, this->inputs[1]);
    }

    SuggestExtensions::SuggestExtensions(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction(matlabEngine, storage, u"suggest_extensions")
    {
        this->min_inputs = this->max_inputs = 2;
        this->min_outputs = this->max_outputs = 1;
    }


    void SuggestExtensions::extra_input_checks(SuggestExtensionsParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a moment matrix.");
        }
    }

    void SuggestExtensions::operator()(IOArgumentRange output, SuggestExtensionsParams &input) {
        // Get matrix system, and check it is of the right type
        auto matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;
        const auto* imsPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&matrixSystem);
        if (imsPtr == nullptr) {
            throw_error(matlabEngine, errors::bad_param,
                        "Supplied system key was not to an inflation matrix system.");
        }
        const auto& inflationMatrixSystem = *imsPtr;

        // Lock to read, get operator matrix
        auto lock = inflationMatrixSystem.get_read_lock();
        const auto& operatorMatrix = getMatrixOrThrow(this->matlabEngine, matrixSystem, input.matrix_index);

        const auto* mmPtr = dynamic_cast<const MomentMatrix*>(&operatorMatrix);
        if (mmPtr == nullptr) {
            throw_error(matlabEngine, errors::bad_param,
                        "Currently extensions can only be suggested for moment matrices.");
        }
        const auto& momentMatrix = *mmPtr;
        auto extensions = inflationMatrixSystem.suggest_extensions(momentMatrix);

        // Print output
        matlab::data::ArrayFactory factory;
        output[0] = write_as_array<uint64_t>(factory, extensions.cbegin(), extensions.cend());

    }
}