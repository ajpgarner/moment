/**
 * commutator.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "commutator.h"
#include "binary_operation_impl.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"


namespace Moment::mex::functions {

    // Explicitly instantiate binary operation functions:
    template class BinaryOperation<CommutatorParams, MTKEntryPointID::Commutator>;

    CommutatorParams::CommutatorParams(SortedInputs &&structuredInputs)
        : BinaryOperationParams{std::move(structuredInputs)} {

        // Check if commuting or anticommuting
        if (this->flags.contains(u"commute")) {
            this->anticommute = false;
        } else if (this->flags.contains(u"anticommute")) {
            this->anticommute = true;
        }

        // Additional type checking (for now!)
        switch (this->product_type()) {
            case ProductType::OneToOne:
            case ProductType::OneToMany:
            case ProductType::ManyToOne:
            case ProductType::ManyToMany:
                break;

            default:
                throw_error(matlabEngine, errors::bad_param,
                            "Unsupported input format for (anti)commutator...");

        }
    }


    void Commutator::additional_setup(CommutatorParams& input) {
        if (const auto * pms_ptr = dynamic_cast<const Pauli::PauliMatrixSystem*>(this->ms_ptr); pms_ptr != nullptr) {
            this->pauli_context_ptr = &pms_ptr->pauliContext;
        } else {
            throw_error(this->matlabEngine, errors::bad_param,
                        "Currently (anti)commutators are only supported for the Pauli scenario.");
        }
        this->anticommute = input.anticommute;
    }

    RawPolynomial Commutator::one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) {
        assert(this->pauli_context_ptr != nullptr);
        if (this->anticommute) {
            return this->pauli_context_ptr->anticommutator(lhs, rhs);
        } else {
            return this->pauli_context_ptr->commutator(lhs, rhs);
        }
    }
}