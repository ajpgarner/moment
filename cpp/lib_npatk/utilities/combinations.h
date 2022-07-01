/**
 * combinations.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <cassert>
#include <vector>

namespace NPATK {

    class CombinationIndexIterator {
    public:
        using index_list = std::vector<size_t>;

        /** Total number of objects in set. */
        const size_t N;

        /** Number of objects to choose from. */
        const size_t K;

    private:
        /** Indices */
        index_list indices;

        /** True if iter is in end state */
        bool endState;

    public:

        /** Construct iterator in begin state */
        constexpr CombinationIndexIterator(size_t SetSize, size_t SubsetSize)
            : N(SetSize), K(SubsetSize), endState(false) {
            // Initialize as lowest object (K-1, ... 2, 1, 0)
            this->indices.reserve(K);
            for (size_t i = 0; i < K; ++i) {
                indices.push_back(K - i - 1);
            }
        }

        constexpr CombinationIndexIterator(size_t SetSize, size_t SubsetSize, bool)
                : N(SetSize), K(SubsetSize), endState(true) { }

        constexpr CombinationIndexIterator& operator++() {
            assert(!this->endState);

            incIndex(K-1);

            return *this;
        }

        [[nodiscard]] constexpr CombinationIndexIterator operator++(int) & {
            auto copy{*this};
            ++(*this);
            return copy;
        }


        constexpr bool operator==(const CombinationIndexIterator& other) const noexcept {
            // All expired iterators are equivalent, and distinct from unexpired iterators
            if (this->endState != other.endState) {
                return false;
            }
            if (this->endState && other.endState) {
                return true;
            }

            for (std::size_t j = 0; j < K; ++j) {
                if (this->indices[j] != other.indices[j]) {
                    return false;
                }
            }
            return true;
        }

        constexpr bool operator!=(const CombinationIndexIterator& other) const noexcept {
            return !(this->operator==(other));
        }

        constexpr const index_list& operator*() const {
            assert(!this->endState);
            return indices;
        }

        /** True if no more combinations */
        [[nodiscard]] constexpr bool done() const noexcept {
            return this->endState;
        }

    private:
        inline void incIndex(size_t J) { // NOLINT(misc-no-recursion)
            indices[J]++;
            if (J > 0) {
                if (indices[J] >= indices[J - 1]) {
                    indices[J] = K - J - 1;
                    incIndex(J - 1);
                }
            } else {
                if (indices[0] >= N) {
                    endState = true;
                }
            }
        }
    };

}