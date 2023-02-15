/**
 * combinations.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>

#include <algorithm>
#include <concepts>
#include <vector>

namespace Moment {

    /**
     * How many ways of choosing a subset of size K from a set of size N.
     * @tparam int_t The integer type
     * @param N The size of the set
     * @param K The size of the subset
     * @return The number of distinct combinations.
     */
    template<std::integral int_t>
    [[nodiscard]] constexpr int_t combinations(int_t N, int_t K) {
        int_t numerator = 1;
        for (int_t iN = N-K+1; iN <= N; ++iN) {
            numerator *= iN;
        }
        int_t denominator = 1;
        for (int_t iD = 1; iD <= K; ++iD) {
            denominator *= iD;
        }
        return numerator/denominator;
    }

    template<bool inclusive = false>
    class CombinationIndexIteratorBase {
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
        constexpr CombinationIndexIteratorBase(size_t SetSize, size_t SubsetSize)
            : N{SetSize}, K{SubsetSize}, endState{false} {
            assert(SetSize >= SubsetSize);

            // Initialize as lowest object (0, 1, ... K-1)
            this->indices.reserve(K);
            if constexpr (inclusive) {
                std::fill_n(std::back_inserter(this->indices), K, 0);
            } else {
                for (size_t i = 0; i < K; ++i) {
                    this->indices.push_back(i);
                }
            }
        }

        constexpr CombinationIndexIteratorBase(size_t SetSize, size_t SubsetSize, bool set_to_true)
                : N(SetSize), K(SubsetSize), endState(true) {
            assert(set_to_true);
        }

        constexpr CombinationIndexIteratorBase& operator++() {
            assert(!this->endState);

            // K = 0 expires immediately
            if (K == 0) {
                [[unlikely]]
                this->endState = true;
                return *this;
            }

            // Otherwise, increase indices
            if constexpr(inclusive) {
                incIndexInclusive(K-1);
            } else {
                incIndexExclusive(0);
            }

            return *this;
        }

        [[nodiscard]] constexpr CombinationIndexIteratorBase operator++(int) & {
            auto copy{*this};
            ++(*this);
            return copy;
        }


        constexpr bool operator==(const CombinationIndexIteratorBase& other) const noexcept {
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

        constexpr bool operator!=(const CombinationIndexIteratorBase& other) const noexcept {
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

        [[nodiscard]] constexpr size_t combination_count() const noexcept {
            return combinations(this->N, this->K);
        }

    private:
        constexpr void incIndexInclusive(size_t J) { // NOLINT(misc-no-recursion)
            indices[J]++;
            if (J > 0) {
                if (indices[J] >= N) {
                    incIndexInclusive(J - 1);
                    indices[J] = indices[J-1];
                }
            } else {
                // N+1, ... N
                if (indices[0] >= N) {
                    endState = true;
                }
            }
        }

        constexpr void incIndexExclusive(size_t J) { // NOLINT(misc-no-recursion)
            indices[J]++;
            if (J + 1 < K) {
                if (indices[J] >= indices[J + 1]) {
                    indices[J] = J; // minval
                    incIndexExclusive(J + 1);
                }
            } else {
                if (indices[K - 1] >= N) {
                    endState = true;
                }
            }
        }
    };

    using CombinationIndexIterator = CombinationIndexIteratorBase<false>;

    using CommutingIndexIterator = CombinationIndexIteratorBase<true>;

    class PartitionIterator {
    public:
        const size_t N;
        const size_t K;
        const size_t NminusK;

    private:
        CombinationIndexIterator primaryIter;
        std::vector<size_t> complementIndices;
        std::vector<bool> bitField;
        bool is_done;
        using vecref_pair_t = std::pair<const std::vector<size_t>&, const std::vector<size_t>&>;


    public:
        PartitionIterator(size_t SetSize, size_t SubsetSize)
            : N{SetSize}, K{SubsetSize}, NminusK(SetSize - SubsetSize),
               primaryIter(SetSize, SubsetSize), is_done(primaryIter.done()),
               bitField(N, false)
        {
            assert(SetSize >= SubsetSize);
            this->complementIndices.reserve(NminusK);
            for (size_t index = K; index < N; ++index) {
                this->complementIndices.push_back(index); // K, K+1, ... N-1
            }
            std::fill_n(this->bitField.begin(), K, true);
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

        /**
         * Bit field, where element i is true if i is in primary, and false if in i is in complement.
         */
        [[nodiscard]] const auto& bits() const noexcept { return this->bitField; }
        [[nodiscard]] bool bits(size_t index) const noexcept {
            assert(index < N);
            return this->bitField[index];
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
                // Written all complementary values already, no need to do final part of loop
                if (compIndexIter == compIndexIterEnd) {
                    std::fill(this->bitField.begin() + static_cast<ptrdiff_t>(i), this->bitField.end(), true);
                    break;
                }

                // At end of primary, just keep writing until done...
                if (primIndexIter == primIndexIterEnd) {
                    *compIndexIter = i;
                    ++compIndexIter;
                    this->bitField[i] = false;
                } else {
                    if (*primIndexIter == i) {
                        ++primIndexIter; // index is in primary iterator, don't write to complement.
                        this->bitField[i] = true;
                    } else {
                        *compIndexIter = i; // index not in primary iterator, write to complement.
                        ++compIndexIter;
                        this->bitField[i] = false;
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