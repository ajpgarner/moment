/**
 * operator_sequence.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_sequence.h"

#include "scenarios/context.h"

#include <algorithm>
#include <iostream>

namespace Moment {


    std::ostream& operator<<(ContextualOS& os, const OperatorSequence &seq) {
        assert(&os.context == seq.context);
        seq.context->format_sequence(os, seq);
        return os;
    }

    std::ostream& operator<<(std::ostream &os, const OperatorSequence &seq) {
        ContextualOS contextual_os{os, *seq.context};
        seq.context->format_sequence(contextual_os, seq);
        return os;
    }

    std::string OperatorSequence::formatted_string() const {
        return this->context->format_sequence(*this);
    }

    OperatorSequence::OperatorSequence(sequence_storage_t operators,
                                       const Context &context,
                                       const SequenceSignType sign_type) noexcept
        : HashedSequence{std::move(operators), context.hash(operators), sign_type}, context{&context} {
        this->to_canonical_form();
    }

    OperatorSequence::OperatorSequence(const ConstructPresortedFlag&,
                                       sequence_storage_t operators,
                                       const Context &context,
                                       const SequenceSignType sign_type) noexcept
        : HashedSequence{std::move(operators), context.hash(operators), sign_type}, context{&context} {

    }


    void OperatorSequence::to_canonical_form() noexcept {
        // Contextual simplifications
        bool simplify_to_zero = this->context->additional_simplification(this->operators, this->sign);
        if (simplify_to_zero) {
            this->operators.clear();
            this->the_hash = 0;
            this->sign = SequenceSignType::Positive;
            return;
        }

        // Rehash sequence
        this->the_hash = this->context->hash(*this);
    }

    OperatorSequence OperatorSequence::conjugate() const {
        return this->context->conjugate(*this);
    }

    HermitianType OperatorSequence::hermitian_type() const {
        if (this->empty()) {
            // Special case zero:
            if (this->zero()) {
                return HermitianType::Zero;
            }

            // Factor of identity; so as Hermitian as its sign...
            return is_imaginary(this->sign) ? HermitianType::AntiHermitian : HermitianType::Hermitian;
        }

        // Otherwise, calculate conjugate and compare...
        const auto conjugate = this->context->conjugate(*this);
        const auto csm = this->compare_same_negation(*this, conjugate);
        if (1 == csm) {
            return HermitianType::Hermitian;
        } else if (-1 == csm) {
            return HermitianType::AntiHermitian;
        } else {
            return HermitianType::NotHermitian;
        }
    }

    OperatorSequence &OperatorSequence::operator*=(const OperatorSequence &rhs) {
        *this = context->multiply(*this, rhs);
        return *this;
    }

    OperatorSequence operator*(const OperatorSequence &lhs, const OperatorSequence &rhs) {
        assert(lhs.is_same_context(*rhs.context));
        return lhs.context->multiply(lhs, rhs);
    }


}