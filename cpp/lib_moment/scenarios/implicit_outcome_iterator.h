/**
 * implicit_outcome_iterator.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <span>
#include <vector>

#include "outcome_index_iterator.h"


namespace Moment {

    /** Filter OutcomeIndexIterator to only produce strings of a particular implicit number */
    class ImplicitOutcomeIterator {
    private:
        OutcomeIndexIterator rawIter;
        size_t num_implicit;
        bool is_end;

    public:
        ImplicitOutcomeIterator(std::vector<size_t> outcomes_per_measurement, size_t num_implicit, bool end = false);

        ImplicitOutcomeIterator(std::span<const size_t> outcomes_per_measurement, size_t num_implicit, bool end = false)
            : ImplicitOutcomeIterator(std::vector<size_t>(outcomes_per_measurement.begin(), outcomes_per_measurement.end()),
                                      num_implicit, end) { }

        /** Global index, taking into account skipped symbols */
        [[nodiscard]] inline size_t global() const noexcept { return this->rawIter.global(); }

        ImplicitOutcomeIterator& operator++() noexcept;

        [[nodiscard]] ImplicitOutcomeIterator operator++(int) & noexcept {
            ImplicitOutcomeIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] inline auto operator*() const noexcept {
            return *this->rawIter;
        }

        [[nodiscard]] inline bool operator==(const ImplicitOutcomeIterator& rhs) const noexcept {
            return this->rawIter == rhs.rawIter;
        }

        [[nodiscard]] inline bool operator!=(const ImplicitOutcomeIterator& rhs) const noexcept {
            return this->rawIter != rhs.rawIter;
        }

        [[nodiscard]] inline bool done() const noexcept {
            return this->is_end;
        }

    };

}