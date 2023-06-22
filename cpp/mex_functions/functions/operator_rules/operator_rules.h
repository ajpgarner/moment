/**
 * operator_rules.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex_function.h"

namespace Moment::mex::functions  {

    struct OperatorRulesParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

    public:
        explicit OperatorRulesParams(SortedInputs&& inputs);
    };

    class OperatorRules : public ParameterizedMexFunction<OperatorRulesParams, MEXEntryPointID::OperatorRules> {
    public:
        explicit OperatorRules(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, OperatorRulesParams &input) override;

        void extra_input_checks(OperatorRulesParams &input) const override;

    };

}
