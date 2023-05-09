/**
 * multi_partition.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "small_vector.h"

#include <concepts>
#include <cassert>

namespace Moment {

    /**
     * Iterate over vectors of length P that sum to S.
     * @tparam int_t Integral type of S.
     * @tparam reversed_indices True, if prioritizing population in left-most index.
     */
    template<std::integral int_t = int, bool reversed_indices = false>
    class MultipartitionIterator {
    public:
        using cursor_t = SmallVector<int_t, 4>;

    public:
        /** The number that the indices sum to */
        const int_t Sum;

        /** The number of partitions (and hence number of constituent indices). */
        const size_t Parties;

    private:
        /** The iterating indices.*/
        cursor_t cursor;

        /**
         * Cumulative sum.
         * From right-to-left if reversed indices is true; otherwise from left-to-right.
         */
        cursor_t cum_sum;

        /** True if iterator has completed. */
        bool end_state = false;

    public:
        /**
         * Construct a multi-partition iterator.
         * @param sum The elements add up to this number.
         * @param parties The number of elements.
         */
        constexpr MultipartitionIterator(const int_t sum, const size_t parties)
            : Sum{sum}, Parties{parties},
              cursor(Parties, static_cast<int_t>(0)),
              cum_sum(Parties, reversed_indices ? sum : static_cast<int_t>(0)) {

            assert(this->Sum>=0);
            assert(this->Parties>0);

            if constexpr (reversed_indices) {
                this->cursor[0] = this->Sum;
            } else {
                this->cursor[parties - 1] = this->Sum;
                this->cum_sum[parties - 1] = this->Sum;
            }
        }

        inline MultipartitionIterator& operator++() {
            // Must not be at end:
            assert(*this);

            if constexpr (reversed_indices) {
                inc_reverse();
            } else {
                inc_forward();
            }

            return *this;
        }

        void inc_forward() {

            // Attempt to add one to penultimate index
            ptrdiff_t inc_idx =  this->Parties - 2;

            bool iterating = true;
            while (iterating) {
                // Nothing further can be incremented
                if (inc_idx < 0) {
                    this->end_state = true;
                    return;
                }

                ++this->cursor[inc_idx];
                ++this->cum_sum[inc_idx];
                if (this->cum_sum[inc_idx] > this->Sum) {
                    this->cursor[inc_idx] = 0;
                    --inc_idx;
                }  else {
                    iterating = false;
                }
            }

            // Last element fills to make total
            for (;inc_idx < this->Parties - 1; ++inc_idx) {
                this->cum_sum[inc_idx] = ((inc_idx > 0) ? this->cum_sum[inc_idx-1] : 0) + this->cursor[inc_idx];
            }

            this->cursor[this->Parties - 1] = this->Sum - this->cum_sum[this->Parties - 2];
        };


        void inc_reverse() {
            // Must not be at end:
            assert(*this);

            // Only 0 0... 0 adds up to 0.
            if ((this->Sum == 0) || (this->Parties <= 1)) {
                this->end_state = true;
                return;
            }

            // Move from right-most pile to left by one, but only if smaller

            // Get number one less
            ptrdiff_t drop_idx = this->Parties - 1;

            // Find right-most non-zero entry
            while (this->cursor[drop_idx] == 0) {
                --drop_idx;
                assert(drop_idx >= 0); // cursor summing to pos value implies one or more non-zero values.
            }

            // Final entry is right-most, so we have to do a row-reset
            if (drop_idx == this->Parties-1) {
                ptrdiff_t next_drop_idx = drop_idx-1;
                while (this->cursor[next_drop_idx] == 0) {
                    --next_drop_idx;
                    // cursor summing to pos value implies one or more non-zero values, except at end.
                    if (next_drop_idx == -1) {
                        this->end_state = true;
                        return;
                    }
                }
                --this->cursor[next_drop_idx];
                --this->cum_sum[next_drop_idx];

                ++next_drop_idx;
                this->cursor[next_drop_idx] = this->Sum - this->cum_sum[next_drop_idx-1];
                this->cum_sum[next_drop_idx] = this->Sum;

                ++next_drop_idx;
                for (;next_drop_idx < this->Parties; ++next_drop_idx) {
                    this->cursor[next_drop_idx] = 0;
                    this->cum_sum[next_drop_idx] = this->Sum;
                }

            } else {
                // Reassign
                --this->cursor[drop_idx];
                --this->cum_sum[drop_idx];
                ++this->cursor[drop_idx+1];
            }
        };

        inline const cursor_t& operator*() const noexcept {
            // Must not be at end:
            assert(*this);

            return this->cursor;
        }

        [[nodiscard]] inline int_t operator[](const size_t idx) const noexcept {
            assert(idx < this->Parties);
            return this->cursor[idx];
        }

        /**
         * True if not at end.
         * @return
         */
        [[nodiscard]] constexpr explicit operator bool() const noexcept {
            return !this->end_state;
        }

        bool operator==(const MultipartitionIterator& rhs) const noexcept {
            assert(this->Parties == rhs.Parties);

            if (this->end_state) {
                return rhs.end_state;
            } else if (rhs.end_state) {
                return false;
            }

            for (size_t idx = 0; idx < this->Parties; ++idx) {
                if (this->cursor[idx] != rhs.cursor[idx]) {
                    return false;
                }
            }
            return true;
        }

        inline bool operator!=(const MultipartitionIterator& rhs) const noexcept {
            return !(this->operator==(rhs));
        }

    };
}