/**
 * commutator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "../../mtk_function.h"

#include "integer_types.h"
#include "import/algebraic_operand.h"

namespace Moment::mex::functions {

    struct CommutatorParams : public SortedInputs {
    public:
        uint64_t matrix_system_key = 0;
        bool anticommute = false;

        AlgebraicOperand lhs;
        AlgebraicOperand rhs;

    public:
        explicit CommutatorParams(SortedInputs&& inputs);
    };

    class Commutator : public ParameterizedMTKFunction<CommutatorParams, MTKEntryPointID::Commutator> {
    public:
        explicit Commutator(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, CommutatorParams &input) override;

        void extra_input_checks(CommutatorParams &input) const override;

    };
}