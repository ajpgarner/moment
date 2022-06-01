/**
 * hermitian_operator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "operator_sequence.h"

#include <algorithm>
#include <iostream>

std::ostream &NPATK::operator<<(std::ostream &os, const NPATK::Operator &op) {
    os << op.party.id << "_" << op.id;
    return os;
}


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
    // Group first by party (preserving ordering within each party)
    std::stable_sort(this->constituents.begin(), this->constituents.end(),
                     Operator::PartyComparator{});

    // Remove idempotent elements.
    auto trim = std::unique(this->constituents.begin(), this->constituents.end(),
                     Operator::IsRedundant{});
    this->constituents.erase(trim, this->constituents.end());
}

NPATK::OperatorSequence NPATK::OperatorSequence::conjugate() const {
    OperatorSequence output{};
    output.constituents.reserve(this->constituents.size());

    std::copy(this->constituents.crbegin(), this->constituents.crend(),
              std::back_inserter(output.constituents));

    // Do commutation, etc.
    output.to_canonical_form();

    return output;
}
