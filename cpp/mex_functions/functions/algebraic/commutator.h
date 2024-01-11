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
#include "import/matrix_system_id.h"

namespace Moment::Pauli {
    class PauliMatrixSystem;
}

namespace Moment::mex::functions {

    struct CommutatorParams : public SortedInputs {
    public:
        /** Key to the matrix system. */
        MatrixSystemId matrix_system_key;

        /** True to calculate anticommutator, false for commutator */
        bool anticommute = false;

        /** Left hand operand */
        AlgebraicOperand lhs;

        /** Right hand operand */
        AlgebraicOperand rhs;

        /** Type of operation requested */
        ProductType resolved_product_type = ProductType::Incompatible;

    public:
        explicit CommutatorParams(SortedInputs&& inputs);

    };

    class Commutator : public ParameterizedMTKFunction<CommutatorParams, MTKEntryPointID::Commutator> {
    public:
        explicit Commutator(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void operator()(IOArgumentRange output, CommutatorParams &input) override;
    };
}