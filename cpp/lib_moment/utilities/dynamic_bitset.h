/**
 * dynamic_bitset.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "dynamic_bitset_fwd.h"

#include <cassert>
#include <cstdint>

#include <algorithm>
#include <bit>
#include <concepts>
#include <limits>
#include <numeric>
#include <set>
#include <vector>

namespace Moment {

    template<std::unsigned_integral page_t, std::integral index_t, typename storage_t>
    class DynamicBitset {
    public:
        using Index = index_t;

        class DynamicBitsetIterator {
        private:
            DynamicBitset<page_t, index_t, storage_t> cursor;

        public:
            /** (Generally) non-empty iterator */
            constexpr explicit DynamicBitsetIterator(const DynamicBitset<page_t, index_t, storage_t>& val)
                : cursor{val} { }

            /** Empty iterator; "end" */
            constexpr explicit DynamicBitsetIterator(index_t bit_size) : cursor{bit_size} { }

            [[nodiscard]] constexpr index_t operator*() const noexcept {
                return this->cursor.first_index();
            }

            constexpr DynamicBitsetIterator& operator++() {
                auto index = this->cursor.first_index();
                assert(index < this->cursor.bit_size);
                this->cursor.unset(index);
                return *this;
            }

            [[nodiscard]] constexpr bool operator==(const DynamicBitsetIterator& other) const noexcept {
                return this->cursor == other.cursor;
            }

            [[nodiscard]] constexpr bool operator!=(const DynamicBitsetIterator& other) const noexcept {
                return this->cursor != other.cursor;
            }
        };

        class ElementProxy {
        private:
            DynamicBitset& bs;
            const index_t index;

        public:
            constexpr explicit ElementProxy(DynamicBitset& bs, const index_t index)
                : bs{bs}, index{index} { }

            constexpr ElementProxy& operator=(const bool rhs) {
                if (rhs) {
                    bs.set(index);
                } else {
                    bs.unset(index);
                }
                return *this;
            }

            constexpr operator bool() const noexcept {
                return bs.test(index);
            }
        };

    public:
        const index_t bit_size;
        const index_t page_count;

        constexpr static const index_t page_size = std::numeric_limits<page_t>::digits;

    private:
        const page_t final_page_mask;
        storage_t data;

    public:
        /** Construct an empty dynamic bitset */
        constexpr explicit DynamicBitset(const index_t bit_size)
            : bit_size{bit_size}, page_count{pages_required(bit_size)},
              final_page_mask{make_final_mask(bit_size)}, data(page_count, static_cast<page_t>(0)) {
        }

        /** Construct an empty dynamic bitset with all values set to default_value */
        constexpr DynamicBitset(const index_t bit_size, const bool default_value)
            : DynamicBitset<page_t, index_t, storage_t>{bit_size} {
            if (default_value) {
                for (index_t i = 0, iMax = page_count - 1; i < iMax; ++i) {
                    this->data[i] = ~static_cast<page_t>(0);
                }
                this->data[page_count - 1] = this->final_page_mask;
            }
        }

        constexpr DynamicBitset(const DynamicBitset& rhs) = default;

        constexpr DynamicBitset(DynamicBitset&& rhs) noexcept = default;

        constexpr void swap(DynamicBitset& rhs) noexcept {
            assert(this->bit_size == rhs.bit_size);
            this->data.swap(rhs.data);
        }

        constexpr void set(const index_t index) noexcept {
            const auto [page, bit] = this->unfold_index(index);
            this->data[page] = this->data[page] | (static_cast<page_t>(1) << bit);
        }

        constexpr void unset(const index_t index) noexcept {
            const auto [page, bit] = this->unfold_index(index);
            this->data[page] = this->data[page] & ~(static_cast<page_t>(1) << bit);
            // So long as index is always in bound, no need for special masking for final page
        }

        constexpr void clear() noexcept {
            for (index_t i = 0, iMax = page_count; i < iMax; ++i) {
                this->data[i] = static_cast<page_t>(0);
            }
        }

        [[nodiscard]] constexpr bool test(const index_t index) const noexcept {
            const auto [page, bit] = this->unfold_index(index);
            return ((this->data[page] & (static_cast<page_t>(1) << bit)) != 0);
        }

        /** True, if no bits are set */
        [[nodiscard]] constexpr bool empty() const noexcept {
            return std::all_of(this->data.cbegin(), this->data.cend(),
                               [](auto x) { return x == static_cast<page_t>(0);});
        }

        /** Counts number of bits that are set */
        [[nodiscard]] constexpr index_t count() const noexcept {
            return std::transform_reduce(this->data.cbegin(), this->data.cend(), static_cast<index_t>(0), std::plus{},
                                         [](auto x) { return std::popcount(x); });
        }

        /** Gets index of first set bit, or bit_size if no bits are set. */
        [[nodiscard]] constexpr index_t first_index() const noexcept {
            index_t first = 0;
            for (auto page : this->data) {
                auto page_first = static_cast<index_t>(std::countr_zero(page));
                first += page_first;
                if (page_first != std::numeric_limits<page_t>::digits) {
                    return first;
                }
            }
            return this->bit_size;
        }

        /** Gets iterator over bitfield */
        constexpr auto begin() const {
            return DynamicBitsetIterator{*this};
        }

        constexpr auto end() const {
            return DynamicBitsetIterator{this->bit_size};
        }

        /** Export bitset as a std::set of integers */
        template<std::integral int_t = index_t>
        [[nodiscard]] std::set<int_t> to_set() const {
            std::set<int_t> output;
            for (auto x : *this) {
                output.emplace_hint(output.end(), static_cast<int_t>(x));
            }
            return output;
        }


        /**
         * Creates proxy object for assignment, which is castable to bool of elements
         */
        [[nodiscard]] constexpr ElementProxy operator[](index_t index) noexcept {
            return ElementProxy{*this, index};
        }

        /**
         * Alias for test().
         */
        [[nodiscard]] constexpr bool operator[](index_t index) const noexcept {
            return this->test(index);
        }

        [[nodiscard]] constexpr bool
        operator==(const DynamicBitset<page_t, index_t, storage_t>& rhs) const noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (index_t p = 0; p < this->page_count; ++p) {
                if (this->data[p] != rhs.data[p]) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] constexpr bool
        operator!=(const DynamicBitset<page_t, index_t, storage_t>& rhs) const noexcept {
            return !this->operator==(rhs);
        }


        constexpr DynamicBitset<page_t, index_t, storage_t>&
        operator&=(const DynamicBitset<page_t, index_t, storage_t>& rhs) noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (index_t index = 0; index < this->page_count; ++index) {
                this->data[index] &= rhs.data[index];
            }
            return *this;
        }

        [[nodiscard]] constexpr DynamicBitset<page_t, index_t, storage_t>
        operator&(const DynamicBitset<page_t, index_t, storage_t>& rhs) const noexcept {
            DynamicBitset<page_t, index_t, storage_t> copy{*this};
            copy &= rhs;
            return copy;
        }

        constexpr DynamicBitset<page_t, index_t, storage_t>&
        operator|=(const DynamicBitset<page_t, index_t, storage_t>& rhs) noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (index_t index = 0; index < this->page_count; ++index) {
                this->data[index] |= rhs.data[index];
            }
            return *this;
        }

        [[nodiscard]] constexpr DynamicBitset<page_t, index_t, storage_t>
        operator|(const DynamicBitset<page_t, index_t, storage_t>& rhs) const noexcept {
            DynamicBitset<page_t, index_t, storage_t> copy{*this};
            copy |= rhs;
            return copy;
        }

        constexpr DynamicBitset<page_t, index_t, storage_t>& invert_in_place() noexcept {
            // Do nothing if no data
            if (this->page_count <= 0) {
                return *this;
            }

            // Invert
            for (index_t page = 0; page < (this->page_count-1); ++page) {
                this->data[page] = ~data[page];
            }
            this->data[this->page_count - 1] = (~this->data[this->page_count - 1]) & this->final_page_mask;

            return *this;
        }

        /**
         * Copy bitset with all bits inverted
         */
        constexpr DynamicBitset<page_t, index_t, storage_t> operator~() const {
            DynamicBitset<page_t, index_t, storage_t> output{this->bit_size};
            for (index_t page = 0; page < (this->page_count-1); ++page) {
                output.data[page] = ~this->data[page];
            }
            output.data[this->page_count - 1] = (~this->data[this->page_count - 1]) & output.final_page_mask;
            return output;
        }

        /**
         * Construct a subset
         */
        constexpr DynamicBitset subset(const index_t first_element_index, const index_t subset_size) const {
            assert((first_element_index >= 0) && (first_element_index < this->bit_size));
            const index_t last_element_index = (first_element_index + subset_size); // exclusive
            assert((subset_size >= 0) && (last_element_index <= this->bit_size));

            // Prepare output
            DynamicBitset output(subset_size);

            // Early return for empty subset
            if (subset_size == 0) {
                return output;
            }

            const index_t first_input_page = first_element_index / page_size;
            const index_t copy_offset = first_element_index % page_size;

            const index_t last_input_page = last_element_index  / page_size;
            const index_t remainder = last_element_index % page_size;

            if (copy_offset == 0) { // Aligned copy

                // Copy any full pages
                std::copy(this->data.begin() + first_input_page,
                          this->data.begin() + last_input_page,
                          output.data.begin());

                if (remainder != 0) {
                    assert(last_input_page < this->page_count);
                    output.data.back() = *(this->data.begin() + last_input_page) & output.final_page_mask;
                }
            } else { // Unaligned copy
                const auto anti_offset = page_size - copy_offset;

                if ((first_input_page + output.page_count) < this->page_count) {
                    for (index_t out_page = 0; out_page < output.page_count; ++out_page) {
                        output.data[out_page] = (this->data[first_input_page + out_page] >> copy_offset)
                                                    | this->data[first_input_page + out_page + 1] << anti_offset;
                    }
                    output.data.back() &= output.final_page_mask;
                } else {
                    for (index_t out_page = 0; out_page < output.page_count - 1; ++out_page) {
                        output.data[out_page] = (this->data[first_input_page + out_page] >> copy_offset)
                                                    | this->data[first_input_page + out_page + 1] << anti_offset;
                    }
                    output.data.back() = (this->data[first_input_page + output.page_count - 1] >> copy_offset);
                    output.data.back() &= output.final_page_mask;
                }
            }
            return output;
        }


        /**
         * Get a small subset
         */
        constexpr page_t small_subset(const index_t first_element_index, const index_t subset_size) const {
            assert(subset_size <= page_size);
            assert((first_element_index >= 0) && (first_element_index < this->bit_size));
            assert((subset_size >= 0) && (first_element_index + subset_size <= this->bit_size));

            // Empty subset
            if (subset_size == 0) {
                return page_t{0};
            }

            const index_t first_page = first_element_index / page_size;
            const index_t offset = first_element_index % page_size;
            const page_t mask = make_final_mask(subset_size);

            // Aligned subset
            if (offset == 0) {
                return this->data[first_page] & mask;
            }

            // Subset contained entirely within one page
            if (offset + subset_size <= page_size) {
                return (this->data[first_page] >> offset) & mask;
            }

            // Subset spans a page boundary
            const index_t anti_offset = page_size - offset;
            return ((this->data[first_page] >> offset) | (this->data[first_page+1] << anti_offset)) & mask;

        }

    private:
        [[nodiscard]] constexpr std::pair<index_t, index_t> unfold_index(const index_t index) const noexcept {
            assert(index < bit_size);
            const index_t page = index / page_size;
            const index_t bit = index % page_size;
            assert(page < page_count);
            return {page, bit};
        }

    private:
        [[nodiscard]] constexpr static index_t pages_required(const index_t size) noexcept {
            if (0 == (size % page_size)) {
                return size / page_size;
            } else {
                return (size / page_size) + 1;
            }
        }

        [[nodiscard]] constexpr static page_t make_final_mask(const index_t bit_size) noexcept {
            const auto remainder = bit_size % std::numeric_limits<page_t>::digits;
            if (0 == remainder) {
                return ~static_cast<page_t>(0); // 0xff..ff
            }
            return (static_cast<page_t>(1) << (remainder)) - 1; // e.g (1<<63)=0x80...; -1 sets all but the highest bit.
        }


    };


}