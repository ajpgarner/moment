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
                    static_cast<oper_name_t>(input.qubit_count), input.wrap, input.row_width
            );
        }
    }

    PauliMatrixSystemParams::PauliMatrixSystemParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Read number of qubits
        this->qubit_count = read_positive_integer<size_t>(matlabEngine, "Qubit count", this->inputs[0], 0);

        // Optional parameters
        this->find_and_parse(u"tolerance", [this](matlab::data::Array& tol_param) {
            this->zero_tolerance = read_as_double(this->matlabEngine, tol_param);
            if (this->zero_tolerance < 0) {
                throw_error(this->matlabEngine, errors::bad_param, "Tolerance must be non-negative value.");
            }
        });

        // Do we wrap?
        if (this->flags.contains(u"wrap")) {
            this->wrap = true;
        }

        // What about lattice?
        auto col_iter = this->params.find(u"columns");
        const bool has_columns_value = col_iter != this->params.cend();
        if (this->flags.contains(u"lattice") || has_columns_value) {
            if (has_columns_value) {
                this->row_width = read_positive_integer(matlabEngine, "Parameter 'columns'", col_iter->second, 1);
                auto remainder = this->qubit_count % this->row_width;
                if (remainder != 0) {
                    throw_error(matlabEngine, errors::bad_param,
                                "If the 'columns' parameter is set,"
                                " then it must be a factor of the number of qubits.");
                }
            } else {
                double sqrt_qubits = std::sqrt(static_cast<double>(this->qubit_count));
                auto rounded_qubits = static_cast<size_t>(sqrt_qubits);
                if ((rounded_qubits * rounded_qubits) != this->qubit_count) {
                    throw_error(matlabEngine, errors::bad_param,
                                "If 'lattice' flag is set, but column size is not provided,"
                                " then the number of qubits should be a square number.");
                }
                this->row_width = rounded_qubits;
            }
        } else {
            this->row_width = 0;
        }
    }

    PauliMatrixSystem::PauliMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
        : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"lattice");
        this->flag_names.emplace(u"wrap");

        this->param_names.emplace(u"columns");
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