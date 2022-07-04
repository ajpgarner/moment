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
            : N(SetSize), K(SubsetSize), endState(SubsetSize==0) {
            assert(SetSize >= SubsetSize);

            // Initialize as lowest object (0, 1, ... K-1)
            this->indices.reserve(K);
            for (size_t i = 0; i < K; ++i) {
                indices.push_back(i);
            }
        }

        constexpr CombinationIndexIterator(size_t SetSize, size_t SubsetSize, bool)
                : N(SetSize), K(SubsetSize), endState(true) { }

        constexpr CombinationIndexIterator& operator++() {
            assert(!this->endState);

            incIndex(0);

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

        constexpr const index_list& operator*() const noexcept {
            assert(!this->endState);
            return indices;
        }

        constexpr const index_list * operator->() const noexcept {
            assert(!this->endState);
            return &(this->indices);
        }

        /** True if no more combinations */
        [[nodiscard]] constexpr bool done() const noexcept {
            return this->endState;
        }

    private:
        inline void incIndex(size_t J) { // NOLINT(misc-no-recursion)
            indices[J]++;
            if (J < K-1) {
                if (indices[J] >= indices[J + 1]) {
                    indices[J] = J; // minval
                    incIndex(J + 1);
                }
            } else {
                if (indices[K-1] >= N) {
                    endState = true;
                }
            }
        }
    };


    class PartitionIterator {
    private:
        CombinationIndexIterator primaryIter;
        std::vector<size_t> complementIndices;

        bool is_done;

        using vecref_pair_t = std::pair<const std::vector<size_t>&, const std::vector<size_t>&>;

    public:
        const size_t N;
        const size_t K;
        const size_t NminusK;

    public:
        PartitionIterator(size_t SetSize, size_t SubsetSize)
            : primaryIter(SetSize, SubsetSize), is_done(primaryIter.done()),
              N{SetSize}, K{SubsetSize}, NminusK(SetSize - SubsetSize)
        {
            assert(SetSize >= SubsetSize);
            this->complementIndices.reserve(NminusK);
            for (size_t index = K; index < N; ++index) {
                this->complementIndices.push_back(index); // K, K+1, ... N-1
            }
        }

        [[nodiscard]] bool done() const noexcept { return this->is_done; }

        [[nodiscard]] const auto& primary() const noexcept { return *this->primaryIter; }
        [[nodiscard]] size_t primary(size_t index) const noexcept {
            assert(index < K);
            return (*this->primaryIter)[index];
        }

        [[nodiscard]] const auto& complement() const noexcept { return this->complementIndices; }
        [[nodiscard]] size_t complement(size_t index) const noexcept {
            assert(index < NminusK);
            return this->complementIndices[index];
        }

        vecref_pair_t operator*() const noexcept {
            return {this->primary(), this->complement()};
        }

        PartitionIterator& operator++() {
            ++this->primaryIter;
            if (this->primaryIter.done()) {
                this->is_done = true;
                return *this;
            }
            auto compIndexIter = this->complementIndices.begin();
            const auto compIndexIterEnd  = this->complementIndices.end();

            auto primIndexIter = this->primaryIter->cbegin();
            const auto primIndexIterEnd  = this->primaryIter->cend();

            // Algorithm works on assertion that primary index iterator is sorted (this should be by construction true).
            for (size_t i = 0; i < N; ++i) {
                // Written all complementary values already, finish loop
                if (compIndexIter == compIndexIterEnd) {
                    break;
                }

                // At end of primary, just keep writing until done...
                if (primIndexIter == primIndexIterEnd) {
                    *compIndexIter = i;
                    ++compIndexIter;
                } else {
                    if (*primIndexIter == i) {
                        ++primIndexIter; // index is in primary iterator, don't write to complement.
                    } else {
                        *compIndexIter = i; // index not in primary iterator, write to complement.
                        ++compIndexIter;
                    }

                }
            }
            assert(compIndexIter == compIndexIterEnd);
            return *this;
        }

        [[nodiscard]] PartitionIterator operator++(int)& {
            PartitionIterator copy{*this};
            ++(*this);
            return copy;
        }



    };

}