/**
 * hermitian_operator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_sequence.h"
#include "operator_collection.h"

#include <algorithm>
#include <iostream>



std::ostream &NPATK::operator<<(std::ostream &os, const NPATK::OperatorSequence &seq) {
    // Canonically zero sequences
    if (seq.is_zero) {
        os << "[0]";
        return os;
    }

    // Empty sequence represents ID
    if (seq.empty()) {
        os << "[I]";
        return os;
    }

    // Otherwise, print list of operators
    bool done_one = false;
    for (const auto& op : seq) {
        if (done_one) {
            os << ", ";
        }
        os << op;
        done_one = true;
    }
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
