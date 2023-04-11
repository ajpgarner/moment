/**
 * generate_basis.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "generate_basis.h"

#include "storage_manager.h"

#include "matrix/moment_matrix.h"

#include "export/export_basis.h"
#include "export/export_matrix_basis_masks.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

#include <array>
#include <complex>

namespace Moment::mex::functions {

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

    GenerateBasisParams::GenerateBasisParams(Moment::mex::SortedInputs &&structuredInputs)
            : SortedInputs(std::move(structuredInputs)) {
        // Get reference
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);
        this->matrix_index = read_positive_integer<uint64_t>(matlabEngine, "Matrix index", this->inputs[1], 0);

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

    GenerateBasis::GenerateBasis(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
        : ParameterizedMexFunction(matlabEngine, storage, u"generate_basis") {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 3;

        this->flag_names.emplace(u"sparse");
        this->flag_names.emplace(u"dense");
        this->mutex_params.add_mutex(u"dense", u"sparse");

        this->flag_names.emplace(u"cell");
        this->flag_names.emplace(u"monolith");
        this->mutex_params.add_mutex(u"cell", u"monolith");
    }



    void GenerateBasis::extra_input_checks(GenerateBasisParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw_error(matlabEngine, errors::bad_param, "Supplied key was not to a matrix system.");
        }
    }


    void GenerateBasis::operator()(IOArgumentRange output, GenerateBasisParams &input) {
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        } catch (const Moment::errors::persistent_object_error& poe) {
            std::stringstream errSS;
            errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec;
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }

        assert(matrixSystemPtr); // ^-- should throw if not found
        const MatrixSystem& matrixSystem = *matrixSystemPtr;

        auto lock = matrixSystem.get_read_lock();

        const auto& operatorMatrix = [&]() -> const MonomialMatrix& {
            try {
                return matrixSystem[input.matrix_index];
            } catch (const Moment::errors::missing_component& mce) {
                throw_error(matlabEngine, errors::bad_param, mce.what());
            }
        }();


        const auto& matrix_properties = operatorMatrix.SMP();

        const auto basis_type = matrix_properties.Type();
        const bool complex_output = (basis_type == MatrixType::Hermitian) || (basis_type == MatrixType::Complex);

        // Complex output requires two outputs... give warning
        if (!this->quiet && complex_output && (output.size() < 2)) {
            print_to_console(this->matlabEngine,
                 "Matrix is potentially complex, but the imaginary element output has not been bound."
            );
        }

        // Do generation
        if (input.monolithic_output) {
            if (input.sparse_output) {
                auto [sym, anti_sym] = export_sparse_monolith_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (output.size() >= 2) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = export_dense_monolith_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (output.size() >= 2) {
                    output[1] = std::move(anti_sym);
                }
            }
        } else {
            if (input.sparse_output) {
                auto [sym, anti_sym] = export_sparse_cell_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (output.size() >= 2) {
                    output[1] = std::move(anti_sym);
                }
            } else {
                auto [sym, anti_sym] = export_dense_cell_basis(this->matlabEngine, operatorMatrix);
                output[0] = std::move(sym);
                if (output.size() >= 2) {
                    output[1] = std::move(anti_sym);
                }
            }
        }

        // If enough outputs supplied, also provide basis key
        if (output.size() >= 3) {
            output[2] = export_basis_key(this->matlabEngine, matrix_properties);
        }

    }
}
