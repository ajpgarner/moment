/**
 * ordered_permutation.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>
#include <cstdint>
#include <iterator>
#include <vector>

namespace Moment {

    template<class index_t, typename storage_t = std::vector<index_t>>
    class OrderedPermutationIterator {
    public:
        using index_list = storage_t;

        /** Total number of objects in set. */
        const index_t N;

        /** Number of objects to choose from. */
        const index_t K;

    private:
        /** Indices */
        index_list indices;

        /** True if iter is in end state */
        bool endState;

    public:

        /** Construct iterator in begin state */
        constexpr OrderedPermutationIterator(index_t set_size, index_t string_length)
                : N{set_size}, K{string_length},
                  indices(string_length, static_cast<index_t>(0)), endState(false) {
            assert(N>0);
            assert(K>=0);
            assert(indices.size() == string_length);
        }

        constexpr OrderedPermutationIterator(index_t SetSize, index_t SubsetSize, bool)
                : N(SetSize), K(SubsetSize), indices(SubsetSize, static_cast<index_t>(0)), endState(true) { }

        constexpr OrderedPermutationIterator& operator++() {
            assert(!this->endState);

            inc_index();

            return *this;
        }

        [[nodiscard]] constexpr OrderedPermutationIterator operator++(int) & {
            auto copy{*this};
            ++(*this);
            return copy;
        }


        constexpr bool operator==(const OrderedPermutationIterator& other) const noexcept {
            assert(this->N == other.N);
            assert(this->K == other.K);

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

        constexpr bool operator!=(const OrderedPermutationIterator& other) const noexcept {
            return !(this->operator==(other));
        }

        constexpr const index_list& operator*() const noexcept {
            assert(!this->endState);
            return indices;
        }

        constexpr const index_list * operator->() const noexcept {
            assert(!this->endState);
            return &(this->indices);
        }

        constexpr const index_t operator[](size_t index) const noexcept {
            assert(!this->endState);
            assert(index < this->indices.size());
            return this->indices[index];
        }

        /** True if no more combinations */
        [[nodiscard]] constexpr bool done() const noexcept {
            return this->endState;
        }

    private:
        constexpr bool inc_index() {
            if (this->K == 0) {
                [[unlikely]]
                this->endState = true;
                return true;
            }
            assert(this->indices.size() == this->K);

            // Increase...
            bool overrun = false;
            bool iterating = true;
            size_t cursor = this->indices.size() - 1;
            while (iterating) {
                ++this->indices[cursor];
                if (this->indices[cursor] >= this->N) {
                    if (0 == cursor) {
                        this->endState = true;
                        return true;
                    }
                    overrun = true;
                    --cursor;
                } else {
                    iterating = false;
                }
            }

            if (overrun) {
                for (size_t rCursor = cursor+1; rCursor < this->indices.size(); ++rCursor) {
                    this->indices[rCursor] = this->indices[cursor];
                }
            }

            return false;
        }
    };

}