/**
 * echo_operand.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "../../mtk_function.h"

#include "integer_types.h"
#include "import/algebraic_operand.h"


namespace Moment::mex::functions  {

    struct EchoOperandParams : public SortedInputs {
    public:
        explicit  EchoOperandParams(SortedInputs&& inputs);

        uint64_t matrix_system_key = 0;

        AlgebraicOperand operand;

    };

    class EchoOperand : public ParameterizedMTKFunction<EchoOperandParams, MTKEntryPointID::EchoOperand> {
    public:
        explicit EchoOperand(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, EchoOperandParams &input) override;

        void extra_input_checks(EchoOperandParams &input) const override;

    };

}
