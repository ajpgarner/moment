/**
 * hermitian_operator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_sequence.h"
#include "context.h"

#include <algorithm>
#include <iostream>



std::ostream &NPATK::operator<<(std::ostream &os, const NPATK::OperatorSequence &seq) {
    if (seq.context == nullptr) {
        throw std::runtime_error("operator<< requires OperatorSequence to have an associated context.");
    }

    os << seq.context->format_sequence(seq);
    return os;
}

void NPATK::OperatorSequence::to_canonical_form() noexcept {
    // Group first by party (preserving ordering within each party)
    std::stable_sort(this->constituents.begin(), this->constituents.end(),
                     Operator::PartyComparator{});

    // Remove excess idempotent elements.
    auto trim_idem = std::unique(this->constituents.begin(), this->constituents.end(),
                     Operator::IsRedundant{});
    this->constituents.erase(trim_idem, this->constituents.end());


    // Contextual simplifications
    if (this->context != nullptr) {
        auto [trim, simplify_to_zero] = this->context->additional_simplification(this->constituents.begin(),
                                                                        this->constituents.end());
        if (simplify_to_zero) {
            this->constituents.clear();
            this->is_zero = true;
            return;
        }
        this->constituents.erase(trim, this->constituents.end());
    }

    // Remove excess identity elements
    auto trim_id = std::remove_if(this->constituents.begin(), this->constituents.end(),
                                  [](Operator& op) { return op.identity();});
    this->constituents.erase(trim_id, this->constituents.end());

}

NPATK::OperatorSequence NPATK::OperatorSequence::conjugate() const {
    OperatorSequence output{*this};
    std::reverse(output.constituents.begin(), output.constituents.end());
    output.to_canonical_form();
    return output;
}
