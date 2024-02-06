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
            this->pauli_context_ptr = nullptr;
        }
        this->anticommute = input.anticommute;
        this->tolerance = this->ms_ptr->polynomial_factory().zero_tolerance;
    }

    RawPolynomial Commutator::one_to_one(const RawPolynomial& lhs, const RawPolynomial& rhs) {
        // Whole architecture relies on branch prediction...
        //  in theory could refactor if statements outside of loops.
        if (this->pauli_context_ptr) {
            if (this->anticommute) {
                return this->pauli_context_ptr->anticommutator(lhs, rhs, this->tolerance);
            } else {
                return this->pauli_context_ptr->commutator(lhs, rhs, this->tolerance);
            }
        } else {
            const auto lr = this->context_ptr->multiply(lhs, rhs, 0.5*this->tolerance);
            auto rl = this->context_ptr->multiply(rhs, lhs, 0.5*this->tolerance);

            if (this->anticommute) {
                return RawPolynomial::add(lr, rl, this->tolerance);
            } else {
                return RawPolynomial::subtract(lr, rl, this->tolerance);
            }
        }
    }

    Commutator::Commutator(matlab::engine::MATLABEngine& matlabEngine, StorageManager& storage)
            : BinaryOperation<CommutatorParams, MTKEntryPointID::Commutator>{matlabEngine, storage} {
        this->flag_names.emplace(u"commute");
        this->flag_names.emplace(u"anticommute");
        this->mutex_params.add_mutex(u"commute", u"anticommute");
    }
}