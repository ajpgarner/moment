/**
 * commutator.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "binary_operation.h"

namespace Moment::Pauli {
    class PauliContext;
}

namespace Moment::mex::functions {

    struct CommutatorParams : public BinaryOperationParams {
    public:
        /** True to calculate anticommutator, false for commutator */
        bool anticommute = false;

    public:
        explicit CommutatorParams(SortedInputs&& inputs);
    };

    class Commutator : public BinaryOperation<CommutatorParams, MTKEntryPointID::Commutator> {
    private:
        Pauli::PauliContext const * pauli_context_ptr = nullptr;
        bool anticommute = false;
        double tolerance = 1.0;

    public:
        explicit Commutator(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage);

    protected:
        void additional_setup(CommutatorParams& input) final;

        RawPolynomial one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) final;

    };
}