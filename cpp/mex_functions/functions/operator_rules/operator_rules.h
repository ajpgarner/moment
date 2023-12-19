/**
 * operator_rules.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mtk_function.h"
#include "import/matrix_system_id.h"

namespace Moment::mex::functions  {

    struct OperatorRulesParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

    public:
        explicit OperatorRulesParams(SortedInputs&& inputs);
    };

    class OperatorRules : public ParameterizedMTKFunction<OperatorRulesParams, MTKEntryPointID::OperatorRules> {
    public:
        explicit OperatorRules(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, OperatorRulesParams &input) override;
    };

}
