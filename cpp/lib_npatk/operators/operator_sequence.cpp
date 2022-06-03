/**
 * hermitian_operator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_sequence.h"

#include <algorithm>
#include <iostream>



std::ostream &NPATK::operator<<(std::ostream &os, const NPATK::OperatorSequence &seq) {
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
    // Remove identity elements
    auto trim_id = std::remove_if(this->constituents.begin(), this->constituents.end(),
                                  [](Operator& op) { return op.identity();});
    this->constituents.erase(trim_id, this->constituents.end());

    // Group first by party (preserving ordering within each party)
    std::stable_sort(this->constituents.begin(), this->constituents.end(),
                     Operator::PartyComparator{});

    // Remove excess idempotent elements.
    auto trim_idem = std::unique(this->constituents.begin(), this->constituents.end(),
                     Operator::IsRedundant{});
    this->constituents.erase(trim_idem, this->constituents.end());
}

NPATK::OperatorSequence NPATK::OperatorSequence::conjugate() const {
    OperatorSequence output{*this};
    std::reverse(output.constituents.begin(), output.constituents.end());
    output.to_canonical_form();
    return output;
}
