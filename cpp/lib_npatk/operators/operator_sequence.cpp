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

    void OperatorSequence::to_canonical_form() noexcept {
        // Group first by party (preserving ordering within each party)
        std::stable_sort(this->constituents.begin(), this->constituents.end(),
                         Operator::PartyComparator{});

        // Remove excess idempotent elements.
        auto trim_idem = std::unique(this->constituents.begin(), this->constituents.end(),
                                     Operator::IsRedundant{});
        this->constituents.erase(trim_idem, this->constituents.end());


        // Contextual simplifications
        bool simplify_to_zero = this->context.additional_simplification(this->constituents);
        if (simplify_to_zero) {
            this->constituents.clear();
            this->is_zero = true;
            return;
        }

        // Remove excess identity elements
        auto trim_id = std::remove_if(this->constituents.begin(), this->constituents.end(),
                                      [](Operator &op) { return op.identity(); });
        this->constituents.erase(trim_id, this->constituents.end());

    }

    OperatorSequence OperatorSequence::conjugate() const {
        OperatorSequence output{*this};
        std::reverse(output.constituents.begin(), output.constituents.end());
        output.to_canonical_form();
        return output;
    }

}