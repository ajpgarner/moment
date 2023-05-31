/**
 * monomial_rules.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "monomial_rules.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "storage_manager.h"
#include "export/export_monomial_rules.h"

#include "utilities/read_as_scalar.h"
#include "utilities/reporting.h"


namespace Moment::mex::functions {

    MonomialRulesParams::MonomialRulesParams(SortedInputs &&rawInput)
        : SortedInputs(std::move(rawInput)) {
        this->storage_key = read_positive_integer<uint64_t>(matlabEngine, "MatrixSystem reference",
                                                            this->inputs[0], 0);
    }

    MonomialRules::MonomialRules(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : ParameterizedMexFunction{matlabEngine, storage} {
        this->min_outputs = this->max_outputs = 1;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    void MonomialRules::extra_input_checks(MonomialRulesParams &input) const {
        if (!this->storageManager.MatrixSystems.check_signature(input.storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }
    }

    void MonomialRules::operator()(IOArgumentRange output, MonomialRulesParams &input) {
        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const Moment::errors::persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }

        // Attempt to cast to algebraic matrix system
        auto * amsPtr = dynamic_cast<Algebraic::AlgebraicMatrixSystem*>(matrixSystemPtr.get());
        if (nullptr == amsPtr) {
            throw_error(this->matlabEngine, errors::bad_param,
                        "MatrixSystem was not an AlgebraicMatrixSystem");
        }
        const auto& ams = *amsPtr;

        // Get read lock on system
        std::shared_lock lock = ams.get_read_lock();

        // Read rules
        const auto& context = ams.AlgebraicContext();
        const auto& rules = context.rulebook();

        // Output list of parsed rules
        if (output.size() >= 1) {
            output[0] = export_monomial_rules(rules, true);
        }
    }
}