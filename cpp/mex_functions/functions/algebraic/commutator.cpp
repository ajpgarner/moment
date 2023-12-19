/**
 * commutator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "commutator.h"

#include "storage_manager.h"

#include "utilities/reporting.h"
#include "utilities/read_as_scalar.h"

namespace Moment::mex::functions {
    CommutatorParams::CommutatorParams(SortedInputs &&structuredInputs)
            : SortedInputs{std::move(structuredInputs)},
              matrix_system_key{matlabEngine}, lhs{matlabEngine, "LHS"}, rhs{matlabEngine, "RHS"} {
        // Get matrix system reference
        this->matrix_system_key.parse_input(this->inputs[0]);

        // Check type of LHS input
        this->lhs.parse_input(this->inputs[1]);

        // Check type of RHS input
        this->rhs.parse_input(this->inputs[2]);

        // Check if commuting or anticommuting
        if (this->flags.contains(u"commute")) {
            this->anticommute = false;
        } else if (this->flags.contains(u"anticommute")) {
            this->anticommute = true;
        }
    }

    Commutator::Commutator(matlab::engine::MATLABEngine &matlabEngine, StorageManager &storage)
            : ParameterizedMTKFunction{matlabEngine, storage} {
        this->min_inputs = 3;
        this->max_inputs = 3;
        this->min_outputs = 2;
        this->max_outputs = 4;

        this->flag_names.emplace(u"commute");
        this->flag_names.emplace(u"anticommute");

        this->mutex_params.add_mutex(u"commute", u"anticommute");
    }

    void Commutator::operator()(IOArgumentRange output, CommutatorParams& input) {
        throw_error(this->matlabEngine, errors::internal_error, "`commutator` not implemented");
    }

}