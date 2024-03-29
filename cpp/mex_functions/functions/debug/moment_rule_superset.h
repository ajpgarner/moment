/**
 * moment_rule_superset.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "../../mtk_function.h"
#include "import/matrix_system_id.h"

namespace Moment::mex::functions  {

    struct MomentRuleSupersetParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        size_t ruleset_A_index = 0;

        size_t ruleset_B_index = 0;

    public:
        explicit MomentRuleSupersetParams(SortedInputs&& inputs);
    };

    class MomentRuleSuperset : public ParameterizedMTKFunction<MomentRuleSupersetParams,
                                                               MTKEntryPointID::MomentRuleSuperset> {
    public:
        explicit MomentRuleSuperset(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, MomentRuleSupersetParams &input) override;

    };

}