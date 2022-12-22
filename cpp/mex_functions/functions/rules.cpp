/**
 * rules.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "rules.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "storage_manager.h"
#include "fragments/export_monomial_rules.h"
#include "utilities/reporting.h"


namespace Moment::mex::functions {

    RulesParams::RulesParams(matlab::engine::MATLABEngine &matlabEngine, SortedInputs &&inputs) {
        this->storage_key = read_positive_integer(matlabEngine, "MatrixSystem reference", inputs.inputs[0], 0);
    }


    Rules::Rules(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
            : MexFunction(matlabEngine, storage, MEXEntryPointID::Rules, u"rules") {
        this->min_outputs = this->max_outputs = 1;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::unique_ptr<SortedInputs>
    Rules::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        auto output = std::make_unique<RulesParams>(this->matlabEngine, std::move(input));
        // Check key vs. storage manager
        if (!this->storageManager.MatrixSystems.check_signature(output->storage_key)) {
            throw errors::BadInput{errors::bad_signature, "Reference supplied is not to a MatrixSystem."};
        }

        return output;
    }

    void Rules::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        auto& input = dynamic_cast<RulesParams&>(*inputPtr);

        // Get referred to matrix system (or fail)
        std::shared_ptr<MatrixSystem> matrixSystemPtr;
        try {
            matrixSystemPtr = this->storageManager.MatrixSystems.get(input.storage_key);
        } catch(const persistent_object_error& poe) {
            throw_error(this->matlabEngine, errors::bad_param, "Could not find referenced MatrixSystem.");
        }

        // Attempt to cast to algebraic matrix system
        auto * amsPtr = dynamic_cast<AlgebraicMatrixSystem*>(matrixSystemPtr.get());
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