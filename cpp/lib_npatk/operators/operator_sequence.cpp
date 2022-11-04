/**
 * operator_sequence.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_sequence.h"
#include "context.h"

#include <algorithm>
#include <iostream>

namespace NPATK {


    std::ostream& operator<<(std::ostream &os, const NPATK::OperatorSequence &seq) {
        os << seq.context.format_sequence(seq);
        return os;
    }

    OperatorSequence::OperatorSequence(std::vector<oper_name_t> &&operators, const Context &context)
        : HashedSequence(std::move(operators), context.hash(operators)), context{context}
    {
        this->to_canonical_form();
    }


    void OperatorSequence::to_canonical_form() noexcept {
        // Contextual simplifications
        bool simplify_to_zero = this->context.additional_simplification(this->operators);
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
        OperatorSequence output{*this};
        std::reverse(output.operators.begin(), output.operators.end());
        output.to_canonical_form();
        return output;
    }


}