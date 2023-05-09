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


    std::ostream& operator<<(std::ostream &os, const OperatorSequence &seq) {
        os << seq.context.format_sequence(seq);
        return os;
    }

    std::string OperatorSequence::formatted_string() const {
        return this->context.format_sequence(*this);
    }

    OperatorSequence::OperatorSequence(sequence_storage_t operators, const Context &context, const bool negated) noexcept
        : HashedSequence(std::move(operators), context.hash(operators)), context{context}, is_negated{negated}
    {
        this->to_canonical_form();
    }


    void OperatorSequence::to_canonical_form() noexcept {
        // Contextual simplifications
        bool simplify_to_zero = this->context.additional_simplification(this->operators, this->is_negated);
        if (simplify_to_zero) {
            this->operators.clear();
            this->the_hash = 0;
            this->is_zero = true;
            return;
        }
        // Rehash sequence
        this->the_hash = this->context.hash(*this);
    }

    OperatorSequence OperatorSequence::conjugate() const {
        return this->context.conjugate(*this);
    }

    int OperatorSequence::compare_same_negation(const OperatorSequence &lhs, const OperatorSequence &rhs) {
        if (lhs.the_hash != rhs.the_hash) {
            return 0;
        }
        if (lhs.size() != rhs.size()) {
            return 0;
        }
        for (size_t i = 0; i < lhs.size(); ++i) {
            if (lhs[i] != rhs[i]) {
                return 0;
            }
        }
        // Sequences are equal, but are they the same sign?
        return lhs.negated() == rhs.negated() ? 1 : -1;
    }


}