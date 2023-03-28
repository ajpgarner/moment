/**
 * make_representation.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "make_representation.h"

#include "storage_manager.h"

#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "eigen/export_eigen_sparse.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"


namespace Moment::mex::functions {
    MakeRepresentationParams::MakeRepresentationParams(Moment::mex::SortedInputs &&raw_inputs)
            : SortedInputs{std::move(raw_inputs)} {
        // Get matrix system ID
        this->matrix_system_key = read_positive_integer<uint64_t>(matlabEngine, "Reference id", this->inputs[0], 0);

        // Get desired word length
        this->word_length = read_positive_integer<uint64_t>(matlabEngine, "Word length", this->inputs[1], 0);
    }

    MakeRepresentation::MakeRepresentation(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMexFunction(matlabEngine, storage, u"make_representation") {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void MakeRepresentation::extra_input_checks(MakeRepresentationParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.matrix_system_key)) {
            throw errors::BadInput{errors::bad_param, "Invalid or expired reference to MomentMatrix."};
        }
        ParameterizedMexFunction::extra_input_checks(input);
    }

    void MakeRepresentation::operator()(IOArgumentRange output, MakeRepresentationParams &input) {
        using namespace Moment::Symmetrized;

        // Get matrix system (and shared-owning pointer):
        auto msPtr = this->storageManager.MatrixSystems.get(input.matrix_system_key);
        assert(msPtr); // ^- above should throw if absent

        // Cast to symmetrized system
        auto *smsPtr = dynamic_cast<SymmetrizedMatrixSystem*>(msPtr.get());
        if (nullptr == smsPtr) {
            throw_error(matlabEngine, errors::bad_param,
                        "Matrix system reference was not to a symmetrized matrix system.");
        }
        auto& symmetrizedMatrixSystem = *smsPtr;

        // Get symmetry (or throw)
        auto& group = symmetrizedMatrixSystem.group();

        // Make (or retrieve) representation
        auto& representation = group.create_representation(input.word_length);

        // Export to matlab
        if (output.size() >= 1) {
            matlab::data::ArrayFactory factory;
            output[0] = export_eigen_sparse_array(this->matlabEngine, factory, representation.group_elements());
        }
    }
}

