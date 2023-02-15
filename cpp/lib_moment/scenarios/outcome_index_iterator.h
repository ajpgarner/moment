/**
 * outcome_index_iterator.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "utilities/multi_dimensional_index_iterator.h"

#include <span>
#include <vector>

namespace Moment {

    /** Iterate over measurement outcomes */
    class OutcomeIndexIterator {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = MultiDimensionalIndexIterator<false>::value_type;

    private:
        MultiDimensionalIndexIterator<false> indexIter;
        std::vector<bool> is_implicit;
        size_t num_implicit = 0;
        size_t operNumber = 0;


    public:
        explicit OutcomeIndexIterator(std::vector<size_t> outcomes_per_measurement, bool end = false);

        explicit OutcomeIndexIterator(std::span<const size_t> outcomes_per_measurement, bool end = false)
            : OutcomeIndexIterator(std::vector<size_t>(outcomes_per_measurement.begin(),
                                                       outcomes_per_measurement.end()), end) { }

        OutcomeIndexIterator& operator++() noexcept;

        [[nodiscard]] OutcomeIndexIterator operator++(int) & noexcept {
            OutcomeIndexIterator copy{*this};
            ++(*this);
            return copy;
        }

        [[nodiscard]] inline auto operator*() const noexcept {
            return *this->indexIter;
        }

        [[nodiscard]] inline auto operator[](size_t index) const noexcept {
            return this->indexIter.operator[](index);
        }

        /**
         * True if iterator is at end.
         */
        [[nodiscard]] constexpr bool done() const noexcept {
            return indexIter.done();
        }

        /**
         * Vector of bools, indicating which indices do not correspond to explicitly defined operators.
         */
        [[nodiscard]] const auto& implicit() const noexcept {
            return this->is_implicit;
        }

        /**
         * True if index i requires implicit definition.
         */
        [[nodiscard]] bool implicit(size_t i) const noexcept {
            assert(i < this->is_implicit.size());
            return this->is_implicit[i];
        }

        /**
         * If operator is explicitly defined, get the operator's index w.r.t. the (maybe joint) measurement.
         */
        [[nodiscard]] size_t explicit_outcome_index() const noexcept {
            assert(this->num_implicit == 0);
            return this->operNumber;
        }

        /**
         * Number of indices that are "out of bounds" in the CG form.
         */
        [[nodiscard]] const auto& implicit_count() const noexcept {
            return this->num_implicit;
        }

        [[nodiscard]] bool operator==(const OutcomeIndexIterator& rhs) const noexcept {
            return this->indexIter == rhs.indexIter;
        }

        [[nodiscard]] bool operator!=(const OutcomeIndexIterator& rhs) const noexcept {
            return this->indexIter != rhs.indexIter;
        }

        [[nodiscard]] inline size_t global() const noexcept {
            return this->indexIter.global();
        }

    private:
        void check_implicit();
    };
}