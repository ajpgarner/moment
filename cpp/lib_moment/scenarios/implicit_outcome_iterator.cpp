/**
 * implicit_outcome_iterator.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "implicit_outcome_iterator.h"

namespace Moment {
    ImplicitOutcomeIterator::ImplicitOutcomeIterator(std::vector<size_t> opm, size_t num_impl, bool end)
        : rawIter(std::move(opm), end), num_implicit{num_impl}, is_end{end} {
        // Seek until correct number of implicit
        while (!rawIter.done() && (rawIter.implicit_count() != num_implicit)) {
            ++rawIter;
        }
        if (rawIter.done()) {
            this->is_end = true;
        }
    }

    ImplicitOutcomeIterator& ImplicitOutcomeIterator::operator++() noexcept {
        ++rawIter;

        // Seek until correct number of implicit
        while (!rawIter.done() && (rawIter.implicit_count() != num_implicit)) {
            ++rawIter;
        }
        if (rawIter.done()) {
            this->is_end = true;
        }
        return *this;
    }
}
