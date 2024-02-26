/**
 * make_representation.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "make_representation.h"

#include "errors.h"
#include "storage_manager.h"

#include "multithreading/multithreading.h"

#include "scenarios/symmetrized/group.h"
#include "scenarios/symmetrized/symmetrized_matrix_system.h"

#include "eigen/export_eigen_sparse.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"


namespace Moment::mex::functions {
    MakeRepresentationParams::MakeRepresentationParams(Moment::mex::SortedInputs &&raw_inputs)
            : SortedInputs{std::move(raw_inputs)}, matrix_system_key{matlabEngine} {
        // Get matrix system ID
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Get desired word length
        this->word_length = read_positive_integer<size_t>(matlabEngine, "Word length", this->inputs[1], 0);
    }

    MakeRepresentation::MakeRepresentation(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
        : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 2;
        this->max_inputs = 2;
        this->min_outputs = 1;
        this->max_outputs = 1;
    }

    void MakeRepresentation::operator()(IOArgumentRange output, MakeRepresentationParams &input) {
        using namespace Moment::Symmetrized;

        // Get matrix system (and shared-owning pointer):
        auto msPtr = input.matrix_system_key(this->storageManager);
        assert(msPtr); // ^- above should throw if absent

        // Cast to symmetrized system
        auto *smsPtr = dynamic_cast<SymmetrizedMatrixSystem*>(msPtr.get());
        if (nullptr == smsPtr) {
            throw BadParameter{"Matrix system reference was not to a symmetrized matrix system."};
        }
        auto& symmetrizedMatrixSystem = *smsPtr;

        // Get symmetry (or throw)
        auto& group = symmetrizedMatrixSystem.group();

        // Make (or retrieve) representation
        auto mt_policy = this->settings->get_mt_policy();
        auto& representation = group.create_representation(input.word_length, mt_policy);

        // Export to matlab
        if (output.size() >= 1) {
            matlab::data::ArrayFactory factory;
            output[0] = export_eigen_sparse_array(this->matlabEngine, factory, representation.group_elements());
        }
    }
}

