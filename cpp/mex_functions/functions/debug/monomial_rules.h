/**
 * monomial_rules.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "../../mex_function.h"

namespace Moment::mex::functions  {

    struct MonomialRulesParams : public SortedInputs {
    public:
        uint64_t storage_key = 0;

    public:
        explicit MonomialRulesParams(SortedInputs&& inputs);
    };

    class MonomialRules : public ParameterizedMexFunction<MonomialRulesParams, MEXEntryPointID::MonomialRules> {
    public:
        explicit MonomialRules(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MonomialRulesParams &input) override;

        void extra_input_checks(MonomialRulesParams &input) const override;

    };

}
