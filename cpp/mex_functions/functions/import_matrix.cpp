/**
 * import_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "import_matrix.h"

#include "storage_manager.h"

#include "matrix/monomial_matrix.h"

#include "scenarios/imported/imported_matrix_system.h"

#include "import/read_raw_symbol_matrix.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"

namespace Moment::mex::functions {
    ImportMatrixParams::ImportMatrixParams(Moment::mex::SortedInputs &&rawInputs)
       : SortedInputs(std::move(rawInputs)), inputMatrix{std::move(this->inputs[1])} {
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                                  this->inputs[0], 0);

        // Verify form of second argument...!
        auto dimensions = this->inputMatrix.getDimensions();
        if ((dimensions.size() != 2) || (dimensions[0] != dimensions[1])) {
            throw errors::BadInput{errors::bad_param, "Input must be square matrix."};
        }

        // Check explicitly requested type
        if (this->flags.contains(u"hermitian")) {
            this->matrix_is_complex = true;
            this->matrix_is_hermitian = true;
        } else if (this->flags.contains(u"symmetric")) {
            this->matrix_is_complex = false;
            this->matrix_is_hermitian = true;
        } else if (this->flags.contains(u"real")) {
            this->matrix_is_complex = false;
            this->matrix_is_hermitian = false;
        } else if (this->flags.contains(u"complex")) {
            this->matrix_is_complex = true;
            this->matrix_is_hermitian = false;
        }

    }

    ImportMatrix::ImportMatrix(matlab::engine::MATLABEngine &matlabEngine, Moment::mex::StorageManager &storage)
        : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_inputs = this->max_inputs = 2;
        this->min_outputs = this->max_outputs = 1;
        this->flag_names.insert(u"hermitian");
        this->flag_names.insert(u"symmetric");
        this->flag_names.insert(u"real");
        this->flag_names.insert(u"complex");
        this->mutex_params.add_mutex({u"hermitian", u"symmetric", u"real", u"complex"});
    }

    void ImportMatrix::extra_input_checks(ImportMatrixParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
    }

    void ImportMatrix::operator()(IOArgumentRange output, ImportMatrixParams &input) {
        // Attempt to get matrix system, and cast to ImportedMatrixSystem
        std::shared_ptr<MatrixSystem> matrixSystemPtr = [&]() { ;
            try {
                return this->storageManager.MatrixSystems.get(input.matrix_system_key);
            } catch (const Moment::errors::persistent_object_error &poe) {
                std::stringstream errSS;
                errSS << "Could not find MatrixSystem with reference 0x" << std::hex << input.matrix_system_key
                      << std::dec;
                throw_error(this->matlabEngine, errors::bad_param, errSS.str());
            }
        }();

        MatrixSystem& matrixSystem = *matrixSystemPtr;
        auto* imsPtr = dynamic_cast<Imported::ImportedMatrixSystem*>(&matrixSystem);
        if (nullptr == imsPtr) {
            std::stringstream errSS;
            errSS << "MatrixSystem with reference 0x" << std::hex << input.matrix_system_key << std::dec
                  << " was not a valid ImportedMatrixSystem.";
            throw_error(this->matlabEngine, errors::bad_param, errSS.str());
        }
        Imported::ImportedMatrixSystem& ims = *imsPtr;

        // Check consistent type requested
        if (imsPtr->importedContext.real_only()) {
            if (input.matrix_is_complex) {
                if (!this->quiet) {
                    std::stringstream noticeSS;
                    noticeSS << "Complex matrix type was requested, but system is purely real. "
                             << "Matrix will instead be interpreted as real.";
                    print_warning(this->matlabEngine, noticeSS.str());
                }
                input.matrix_is_complex = false;
            }
        }

        // Read input to raw form
        auto raw_sym_mat = read_raw_symbol_matrix(this->matlabEngine, input.inputMatrix);
        assert(raw_sym_mat);

        // Try import
        size_t matrix_index;
        try {
            matrix_index = ims.import_matrix(std::move(raw_sym_mat),
                                             input.matrix_is_complex, input.matrix_is_hermitian);
        } catch (Imported::errors::bad_import_matrix& e) {
            throw_error(this->matlabEngine, errors::bad_param, e.what());
        }

        // Output created matrix ID
        matlab::data::ArrayFactory factory;
        if (output.size() > 0) {
            output[0] = factory.createScalar(matrix_index);
        }
    }
}