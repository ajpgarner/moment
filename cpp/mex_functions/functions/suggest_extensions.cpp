/**
 * suggest_factors.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "suggest_extensions.h"

#include "storage_manager.h"
#include "matrix/operator_matrix/moment_matrix.h"

#include "scenarios/inflation/inflation_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/write_as_array.h"

namespace Moment::mex::functions  {
    namespace {
        std::pair<const MonomialMatrix&, const MomentMatrix&>
        getMomentMatrixOrThrow(matlab::engine::MATLABEngine &matlabEngine,
                               const MatrixSystem& matrixSystem, size_t index) {
            const auto& matrix = [&]() -> const MonomialMatrix& {
                try {
                    return dynamic_cast<const MonomialMatrix &>(matrixSystem[index]);
                } catch (const Moment::errors::missing_component &mce) {
                    throw BadParameter{mce.what()};
                } catch (const std::bad_cast &bce) {
                    throw BadParameter{"Currently extensions can only be suggested for monomial matrices."};
                }
            }();

            const auto* mmPtr = MomentMatrix::to_operator_matrix_ptr(matrix);
            if (mmPtr == nullptr) {
                throw BadParameter{"Currently extensions can only be suggested for moment matrices."};
            }
            return {matrix, *mmPtr};
        }
    }

    SuggestExtensionsParams::SuggestExtensionsParams(SortedInputs &&rawInputs)
        : SortedInputs{std::move(rawInputs)}, matrix_system_key{matlabEngine} {
        this->matrix_system_key.parse_input(this->inputs[0]);
        this->matrix_index = read_as_scalar<uint64_t>(matlabEngine, this->inputs[1]);
    }

    SuggestExtensions::SuggestExtensions(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMTKFunction{matlabEngine, storage}
    {
        this->min_inputs = this->max_inputs = 2;
        this->min_outputs = this->max_outputs = 1;
    }

    void SuggestExtensions::operator()(IOArgumentRange output, SuggestExtensionsParams &input) {
        // Get matrix system, and check it is of the right type
        auto matrixSystemPtr = input.matrix_system_key(this->storageManager);
        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;
        const auto* imsPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&matrixSystem);
        if (imsPtr == nullptr) {
            throw BadParameter{"Supplied system key was not to an inflation matrix system."};
        }
        const auto& inflationMatrixSystem = *imsPtr;

        // Lock to read, get operator matrix
        auto lock = inflationMatrixSystem.get_read_lock();
        auto [symbolMatrix, momentMatrix] = getMomentMatrixOrThrow(this->matlabEngine, matrixSystem, input.matrix_index);
        auto extensions = inflationMatrixSystem.suggest_extensions(symbolMatrix);

        // Print output
        matlab::data::ArrayFactory factory;
        output[0] = write_as_array<uint64_t>(factory, extensions.cbegin(), extensions.cend());

    }
}