/**
 * echo_operand.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "../../mtk_function.h"

#include "import/algebraic_operand.h"
#include "import/matrix_system_id.h"

#include "integer_types.h"


namespace Moment::mex::functions  {

    struct EchoOperandParams : public SortedInputs {
    public:
        explicit  EchoOperandParams(SortedInputs&& inputs);

        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** Algebraic object to echo */
        AlgebraicOperand operand;

        /** Set to true to parse to symbolic Polynomial; false for RawPolynomial */
        bool parse_to_symbols = false;

    };

    class EchoOperand : public ParameterizedMTKFunction<EchoOperandParams, MTKEntryPointID::EchoOperand> {
    public:
        explicit EchoOperand(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, EchoOperandParams &input) override;
    };

}
