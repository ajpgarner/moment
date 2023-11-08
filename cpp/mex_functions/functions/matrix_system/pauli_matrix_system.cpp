/**
 * pauli_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_matrix_system.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include "storage_manager.h"

namespace Moment::mex::functions {

    namespace {
        std::unique_ptr<Pauli::PauliContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                                const PauliMatrixSystemParams &input) {
            return std::make_unique<Pauli::PauliContext>(
                    static_cast<oper_name_t>(input.qubit_count)
            );
        }
    }

    PauliMatrixSystemParams::PauliMatrixSystemParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Read number of qubits
        this->qubit_count = read_positive_integer<size_t>(matlabEngine, "Qubit count", this->inputs[0], 0);

        // Optional parameters
        auto tolerance_param_iter = this->params.find(u"tolerance");
        if (tolerance_param_iter != this->params.cend()) {
            this->zero_tolerance = read_as_double(this->matlabEngine, tolerance_param_iter->second);
            if (this->zero_tolerance < 0) {
                throw_error(this->matlabEngine, errors::bad_param, "Tolerance must be non-negative value.");
            }
        }
    }

    PauliMatrixSystem::PauliMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
        : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->param_names.emplace(u"tolerance");

        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void PauliMatrixSystem::operator()(IOArgumentRange output, PauliMatrixSystemParams& input) {
        // Input to context:
        std::unique_ptr<Pauli::PauliContext> contextPtr{make_context(this->matlabEngine, input)};
        if (!contextPtr) {
            throw_error(this->matlabEngine, errors::internal_error, "Context object could not be created.");
        }

        // Output context in verbose mode
        if (this->verbose) {
            std::stringstream ss;
            ss << "Parsed setting:\n";
            ss << *contextPtr << "\n";
            print_to_console(this->matlabEngine, ss.str());
        }

        // Make new system around context
        std::unique_ptr<MatrixSystem> matrixSystemPtr
                = std::make_unique<Pauli::PauliMatrixSystem>(std::move(contextPtr), input.zero_tolerance);

        // Store context/system
        uint64_t storage_id = this->storageManager.MatrixSystems.store(std::move(matrixSystemPtr));

        // Return reference
        matlab::data::ArrayFactory factory;
        output[0] = factory.createScalar<uint64_t>(storage_id);
    }
}