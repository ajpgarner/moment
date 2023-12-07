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
#include "utilities/read_as_vector.h"
#include "utilities/reporting.h"
#include "utilities/io_parameters.h"

#include "storage_manager.h"

namespace Moment::mex::functions {

    namespace {
        std::unique_ptr<Pauli::PauliContext> make_context(matlab::engine::MATLABEngine &matlabEngine,
                                                                const PauliMatrixSystemParams &input) {
            if (input.lattice_mode) {
                return std::make_unique<Pauli::PauliContext>(input.col_height, input.row_width,
                                                             input.wrap, input.symmetrized);
            } else {
                return std::make_unique<Pauli::PauliContext>(input.qubit_count, input.wrap, input.symmetrized);
            }
        }
    }

    PauliMatrixSystemParams::PauliMatrixSystemParams(SortedInputs &&rawInput)
            : SortedInputs(std::move(rawInput)) {

        // Read number of qubits
        this->read_dimensions_parameter(this->inputs[0]);

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

        // Are we symmetrized
        if (this->flags.contains(u"symmetrized")) {
            if (!this->wrap) {
                throw_error(this->matlabEngine, errors::bad_param, "Symmetrization requires wrapping.");
            }
            this->symmetrized = true;
        }
    }

    void PauliMatrixSystemParams::read_dimensions_parameter(const matlab::data::Array& input) {
        if (1 == input.getNumberOfElements()) {
            this->qubit_count = read_positive_integer<size_t>(matlabEngine, "Qubit count", input, 0);
            this->col_height = this->row_width = 0;
            this->lattice_mode = false;
        } else if (2 == input.getNumberOfElements()) {
            auto lattice_dims = read_positive_integer_array(matlabEngine, "Lattice dimensions", input, 1);
            if (2 != lattice_dims.size()) [[unlikely]] {
                throw_error(this->matlabEngine, errors::bad_param,
                            "Qubit parameter to lattice should be 2-dimensional.");
            }
            this->col_height = lattice_dims[0];
            this->row_width = lattice_dims[1];

            this->qubit_count = this->col_height * this->row_width;
            this->lattice_mode = true;
        } else {
            throw_error(this->matlabEngine, errors::bad_param,
                        "Qubit size parameter should be 1 or 2 dimensional.");
        }
    }

    PauliMatrixSystem::PauliMatrixSystem(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
        : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_outputs = 1;
        this->max_outputs = 1;

        this->flag_names.emplace(u"wrap");
        this->flag_names.emplace(u"symmetrized");
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