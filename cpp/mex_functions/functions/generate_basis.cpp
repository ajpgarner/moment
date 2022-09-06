/**
 * generate_basis.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "generate_basis.h"

#include "detail/make_dense_cell_basis.h"
#include "detail/make_dense_monolith_basis.h"
#include "detail/make_sparse_cell_basis.h"
#include "detail/make_sparse_monolith_basis.h"

#include "storage_manager.h"

#include "operators/matrix/moment_matrix.h"

#include "matlab_classes/moment_matrix.h"

#include "fragments/enumerate_symbols.h"
#include "fragments/export_basis_key.h"
#include "fragments/identify_nonhermitian_elements.h"
#include "fragments/identify_nonsymmetric_elements.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

#include <array>
#include <complex>

namespace NPATK::mex::functions {
    GenerateBasis::GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
        : MexFunction(matlabEngine, storage, MEXEntryPointID::GenerateBasis, u"generate_basis") {
        this->min_inputs = this->max_inputs = 1;
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"symmetric");
        this->flag_names.emplace(u"hermitian");
        this->mutex_params.add_mutex(u"symmetric", u"hermitian");

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"dense", u"sparse");

        this->flag_names.emplace(u"cell");
        this->flag_names.emplace(u"monolith");
        this->mutex_params.add_mutex(u"cell", u"monolith");
    }

    GenerateBasisParams::GenerateBasisParams(matlab::engine::MATLABEngine &matlabEngine,
                                             NPATK::mex::SortedInputs &&structuredInputs)
        : SortedInputs(std::move(structuredInputs)) {

        // Determine input type
        switch(this->inputs[0].getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                this->input_mode = InputMode::MATLABArray;
                this->basis_type = MatrixType::Symmetric;
                break;
            case matlab::data::ArrayType::MATLAB_STRING:
                this->input_mode = InputMode::MATLABArray;
                this->basis_type = MatrixType::Hermitian;
                break;
            case matlab::data::ArrayType::HANDLE_OBJECT_REF:
                this->input_mode = InputMode::MomentMatrixReference;
                this->basis_type = MatrixType::Hermitian;
                break;
            default:
                throw errors::BadInput{errors::bad_param, "Invalid matrix type."};
        }

        // Override basis type
        if (this->flags.contains(u"symmetric")) {
            this->basis_type = MatrixType::Symmetric;
        } else if (this->flags.contains(u"hermitian")) {
            this->basis_type = MatrixType::Hermitian;
        }

        // Set basis type
        if (this->flags.contains(u"cell")) {
            this->monolithic_output = false;
        } else if (this->flags.contains(u"monolith")) {
            this->monolithic_output = true;
        }

        // If an explicitly given array from matlab, check dimensions:
        if (this->input_mode == InputMode::MATLABArray) {
            auto inputDims = this->inputs[0].getDimensions();
            if (inputDims.size() != 2) {
                throw errors::BadInput{errors::bad_param, "Input must be a matrix."};
            }

            if (inputDims[0] != inputDims[1]) {
                throw errors::BadInput{errors::bad_param, "Input must be a square matrix."};
            }

            // Check symmetry / Hermitianity
            if (this->basis_type == MatrixType::Hermitian) {
                if (!is_hermitian(matlabEngine, this->inputs[0])) {
                    throw errors::BadInput{errors::bad_param, "Input must be a Hermitian symbol matrix."};
                }
            } else {
                if (!is_symmetric(matlabEngine, this->inputs[0])) {
                    throw errors::BadInput{errors::bad_param, "Input must be a symmetric symbol matrix."};
                }
            }
        } else if (this->input_mode == InputMode::MomentMatrixReference) {
            auto [mmClassPtr, fail] = read_as_moment_matrix(matlabEngine, inputs[0]); // Implicit copy...
            if (!mmClassPtr) {
                throw errors::BadInput{errors::bad_param, fail.value()};
            }

            this->matrix_system_key = mmClassPtr->SystemKey();
            this->moment_matrix_level = mmClassPtr->Level();
        }

        // Determine sparsity of output
        this->sparse_output = (inputs[0].getType() == matlab::data::ArrayType::SPARSE_DOUBLE);
        if (flags.contains(u"sparse")) {
            this->sparse_output = true;
        } else if (flags.contains(u"dense")) {
            this->sparse_output = false;
        }
    }

    std::unique_ptr<SortedInputs> GenerateBasis::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto output =  std::make_unique<GenerateBasisParams>(this->matlabEngine, std::move(*inputPtr));
        if (output->input_mode == GenerateBasisParams::InputMode::MomentMatrixReference) {
            if (!this->storageManager.MatrixSystems.check_signature(output->matrix_system_key)) {
                throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a moment matrix.");
            }
        }
        return output;
    }

    void GenerateBasis::doGenerateBasisArray(std::array<matlab::data::Array, 3>& output,
                                             GenerateBasisParams& input) {

        auto matrix_properties = enumerate_symbols(this->matlabEngine, input.inputs[0], input.basis_type);
        if (verbose) {
            std::stringstream ss;
            ss << "Found " << matrix_properties.BasisMap().size() << " unique symbols ["
               << matrix_properties.RealSymbols().size() << " with real parts, "
               << matrix_properties.ImaginarySymbols().size() << " with imaginary parts].\n";
            if (debug) {
                ss << "Outputting as " << (input.sparse_output ? "sparse" : "dense") << " basis.\n";
            }
            print_to_console(matlabEngine, ss.str());
        }

        // Make the basis
        if (input.monolithic_output) {
            if (input.sparse_output) {
                auto [sym, anti_sym] = detail::make_sparse_monolith_basis(this->matlabEngine, input.inputs[0],
                                                                      matrix_properties);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = detail::make_dense_monolith_basis(this->matlabEngine, input.inputs[0],
                                                                     matrix_properties);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            }
        } else {
            if (input.sparse_output) {
                auto [sym, anti_sym] = detail::make_sparse_cell_basis(this->matlabEngine, input.inputs[0],
                                                                      matrix_properties);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = detail::make_dense_cell_basis(this->matlabEngine, input.inputs[0],
                                                                     matrix_properties);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            }
        }
        // Provide keys
        output[2] = export_basis_key(this->matlabEngine, matrix_properties);
    }

    void GenerateBasis::doGenerateBasisMomentMatrix(std::array<matlab::data::Array, 3>& output,
                                                    GenerateBasisParams& input) {

        auto matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(matrixSystemPtr); // ^-- should throw if not found

        if (!matrixSystemPtr->HasLevel(input.moment_matrix_level)) {
            throw_error(this->matlabEngine, errors::bad_param,
                        "System has not yet generated a moment matrix at the requested level.");
        }

        const auto& momentMatrix = matrixSystemPtr->CreateMomentMatrix(input.moment_matrix_level);

        const auto& matrix_properties = momentMatrix.SMP();

        //auto output_basis_type = input.basis_type;
        if (input.basis_type == MatrixType::Unknown) {
            input.basis_type = matrix_properties.Type();
        } else if (!this->quiet) {
            // If overrode to symmetric, but matrix might have imaginary elements, give warning:
            if ((MatrixType::Symmetric == input.basis_type)
                && (MatrixType::Hermitian == matrix_properties.Type())) {
                print_to_console(this->matlabEngine, std::string("WARNING: Symmetric basis output was requested, ")
                    + " but some elements of the moment matrix correspond to potentially non-Hermitian operator "
                    + " sequences (i.e. may evaluate to complex values, whose imaginary parts will be ignored).\n");
            }
        }

        if (input.monolithic_output) {
            if (input.sparse_output) {
                auto [sym, anti_sym] = detail::make_sparse_monolith_basis(this->matlabEngine, momentMatrix);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = detail::make_dense_monolith_basis(this->matlabEngine, momentMatrix);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            }
        } else {
            if (input.sparse_output) {
                auto [sym, anti_sym] = detail::make_sparse_cell_basis(this->matlabEngine, momentMatrix);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = detail::make_dense_cell_basis(this->matlabEngine, momentMatrix);
                output[0] = std::move(sym);
                if (input.basis_type == MatrixType::Hermitian) {
                    output[1] = std::move(anti_sym);
                }
            }
        }

        // If enough outputs supplied, also provide keys
        ptrdiff_t key_output = (input.basis_type == MatrixType::Hermitian) ? 2 : 1;
        if (output.size() > key_output) {
            output[key_output] = export_basis_key(this->matlabEngine, matrix_properties);
        }
    }


    void GenerateBasis::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<GenerateBasisParams&>(*inputPtr);

        std::array<matlab::data::Array, 3> outputs{};

        switch (input.input_mode) {
            case GenerateBasisParams::InputMode::MATLABArray:
                doGenerateBasisArray(outputs, input);
                break;
            case GenerateBasisParams::InputMode::MomentMatrixReference:
                doGenerateBasisMomentMatrix(outputs, input);
                break;
            case GenerateBasisParams::InputMode ::Unknown:
            default:
                throw_error(this->matlabEngine, errors::internal_error, "Unknown input type for generate_basis.");
        }

        // Hermitian output requires two outputs...
        if ((input.basis_type == MatrixType::Hermitian) && (output.size() < 2)) {
            throw_error(this->matlabEngine, errors::too_few_outputs,
                                std::string("When generating a Hermitian basis, two outputs are required (one for ")
                                  + "symmetric basis elements associated with the real components, one for the "
                                  + "anti-symmetric imaginary elements associated with the imaginary components.");
        }

        // Symmetric output cannot have three outputs...
        if ((input.basis_type == MatrixType::Symmetric) && (output.size() > 2)) {
            throw_error(this->matlabEngine, errors::too_many_outputs,
                                            std::to_string(output.size())
                                             + " outputs supplied for symmetric basis output, but only"
                                             + " two will be generated (basis, and key).");
        }

        // Move arrays to output
        if (input.basis_type == MatrixType::Hermitian) {
            output[0] = std::move(outputs[0]);
            output[1] = std::move(outputs[1]);
            if (output.size() >= 3) {
                output[2] = std::move(outputs[2]);
            }
        } else {
            output[0] = std::move(outputs[0]);
            if (output.size() >= 2) {
                output[1] = std::move(outputs[2]);
            }
        }

    }
}
