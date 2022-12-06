/**
 * outcome_index_iterator.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "outcome_index_iterator.h"

namespace NPATK {


    OutcomeIndexIterator::OutcomeIndexIterator(std::vector<size_t> outcomes_per_measurement, const bool end)
        : indexIter{std::move(outcomes_per_measurement), end}, is_implicit(indexIter.limits().size(), false) {
        check_implicit();
    }

    void OutcomeIndexIterator::check_implicit() {
        if (this->indexIter.done()) {
            return;
        }

        this->num_implicit = 0;
        const auto& outcome_limits = this->indexIter.limits();
        for (size_t mIndex = 0; mIndex < this->is_implicit.size(); ++mIndex) {
            const bool elemImpl = this->indexIter[mIndex] >= (outcome_limits[mIndex]-1);
            this->is_implicit[mIndex] = elemImpl;
            if (elemImpl) {
                ++this->num_implicit;
            }
        }
    }

    OutcomeIndexIterator& OutcomeIndexIterator::operator++() noexcept {
        ++indexIter;
        check_implicit();
        if (0 == this->num_implicit) {
            ++operNumber;
        }
        return *this;
    }


}