/**
 * generate_basis.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "generate_basis.h"

#include "storage_manager.h"

#include "matrix/moment_matrix.h"

#include "export/export_basis.h"
#include "export/export_matrix_basis_masks.h"
#include "fragments/enumerate_symbols.h"
#include "fragments/identify_nonhermitian_elements.h"
#include "fragments/identify_nonsymmetric_elements.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include <array>
#include <complex>

namespace Moment::mex::functions {

    namespace {
        const SymbolicMatrix& getMatrixOrThrow(matlab::engine::MATLABEngine &matlabEngine,
                                         const MatrixSystem& matrixSystem, size_t index) {
            try {
                return matrixSystem[index];
            } catch (const Moment::errors::missing_component& mce) {
                throw_error(matlabEngine, errors::bad_param, mce.what());
            }
        }
    }

    GenerateBasis::GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
        : MexFunction(matlabEngine, storage, MEXEntryPointID::GenerateBasis, u"generate_basis") {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"symmetric");
        this->flag_names.emplace(u"hermitian");
        this->flag_names.emplace(u"real");
        this->flag_names.emplace(u"complex");
        this->mutex_params.add_mutex({u"symmetric", u"hermitian", u"real", u"complex"});

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"dense", u"sparse");

        this->flag_names.emplace(u"cell");
        this->flag_names.emplace(u"monolith");
        this->mutex_params.add_mutex(u"cell", u"monolith");
    }

    GenerateBasisParams::GenerateBasisParams(matlab::engine::MATLABEngine &matlabEngine,
                                             Moment::mex::SortedInputs &&structuredInputs)
        : SortedInputs(std::move(structuredInputs)) {
        // Get reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);
        this->matrix_index = read_positive_integer<uint64_t>(matlabEngine, "Matrix index", this->inputs[1], 0);

        // Override basis type if necessary
        if (this->flags.contains(u"symmetric")) {
            this->basis_type = MatrixType::Symmetric;
        } else if (this->flags.contains(u"hermitian")) {
            this->basis_type = MatrixType::Hermitian;
        }

        // Choose basis output type
        if (this->flags.contains(u"cell")) {
            this->monolithic_output = false;
        } else if (this->flags.contains(u"monolith")) {
            this->monolithic_output = true;
        }

        // Choose output matrix sparsity
        if (flags.contains(u"sparse")) {
            this->sparse_output = true;
        } else if (flags.contains(u"dense")) {
            this->sparse_output = false;
        }
    }

    std::unique_ptr<SortedInputs> GenerateBasis::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto output = std::make_unique<GenerateBasisParams>(this->matlabEngine, std::move(*inputPtr));
        if (!this->storageManager.MatrixSystems.check_signature(output->matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a moment matrix.");
        }
        return output;
    }


    void GenerateBasis::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<GenerateBasisParams&>(*inputPtr);

        auto matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto lock = matrixSystem.get_read_lock();

        const auto& operatorMatrix = getMatrixOrThrow(this->matlabEngine, matrixSystem, input.matrix_index);
        const auto& matrix_properties = operatorMatrix.SMP();

        if (input.basis_type == MatrixType::Unknown) {
            input.basis_type = matrix_properties.Type();
        } else if (!this->quiet) {
            // If overrode to symmetric, but matrix might have imaginary elements, give warning:
            if ((MatrixType::Symmetric == input.basis_type)
                && (MatrixType::Hermitian == matrix_properties.Type())) {
                print_to_console(this->matlabEngine,
                                 std::string("WARNING: Symmetric basis output was requested, ")
                                            + " but some elements of the moment matrix correspond to potentially non-Hermitian operator "
                                            + " sequences (i.e. may evaluate to complex values, whose imaginary parts will be ignored).\n");
            }
        }

        // Complex output requires two outputs...
        if (input.complex_output() && (output.size() < 2)) {
            throw_error(this->matlabEngine, errors::too_few_outputs,
                        std::string("When generating a Hermitian basis, two outputs are required (one for ")
                        + "symmetric basis elements associated with the real components, one for the "
                        + "anti-symmetric imaginary elements associated with the imaginary components.");
        }

        // Symmetric output cannot have three outputs...
        if ((!input.complex_output()) && (output.size() > 2)) {
            throw_error(this->matlabEngine, errors::too_many_outputs,
                        std::to_string(output.size())
                        + " outputs supplied for symmetric basis output, but only"
                        + " two will be generated (basis, and key).");
        }

        // Do generation
        if (input.monolithic_output) {
            if (input.sparse_output) {
                auto [sym, anti_sym] = export_sparse_monolith_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (input.complex_output()) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = export_dense_monolith_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (input.complex_output()) {
                    output[1] = std::move(anti_sym);
                }
            }
        } else {
            if (input.sparse_output) {
                auto [sym, anti_sym] = export_sparse_cell_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (input.complex_output()) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = export_dense_cell_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (input.complex_output()) {
                    output[1] = std::move(anti_sym);
                }
            }
        }

        // If enough outputs supplied, also provide basis key
        ptrdiff_t key_output = (input.complex_output()) ? 2 : 1;
        if (output.size() > key_output) {
            output[key_output] = export_basis_key(this->matlabEngine, matrix_properties);
        }

    }
}
