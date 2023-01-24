/**
 * dynamic_bitset.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

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

    template<std::unsigned_integral page_t = uint64_t>
    class DynamicBitset {
    public:

        class DynamicBitsetIterator {
        private:
            DynamicBitset<page_t> cursor;

        public:
            /** (Generally) non-empty iterator */
            constexpr explicit DynamicBitsetIterator(const DynamicBitset<page_t>& val) : cursor{val} { }

            /** Empty iterator; "end" */
            constexpr explicit DynamicBitsetIterator(size_t bit_size) : cursor{bit_size} { }

            [[nodiscard]] constexpr size_t operator*() const noexcept {
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

    public:
        const size_t bit_size;
        const size_t page_count;

    private:
        const page_t final_page_mask;
        std::vector<page_t> data;

    public:
        /** Construct an empty dynamic bitset */
        constexpr explicit DynamicBitset(size_t bit_size)
            : bit_size{bit_size}, page_count{pages_required(bit_size)},
              final_page_mask{make_final_mask(bit_size)}, data(page_count, static_cast<page_t>(0)) {
        }

        /** Construct an empty dynamic bitset with all values set to default_value */
        constexpr DynamicBitset(size_t bit_size, bool default_value)
            : DynamicBitset<page_t>{bit_size} {
            if (default_value) {
                for (size_t i = 0, iMax = page_count -1; i < iMax; ++i) {
                    this->data[i] = ~static_cast<page_t>(0);
                }
                this->data[page_count - 1] = final_page_mask;
            }
        }

        constexpr DynamicBitset(const DynamicBitset& rhs) = default;

        constexpr void set(const size_t index) noexcept {
            const auto [page, bit] = this->unfold_index(index);
            this->data[page] = this->data[page] | (static_cast<page_t>(1) << bit);
        }

        constexpr void unset(const size_t index) noexcept {
            const auto [page, bit] = this->unfold_index(index);
            this->data[page] = this->data[page] & ~(static_cast<page_t>(1) << bit);
            // So long as index is always in bound, no need for special masking for final page
        }

        [[nodiscard]] constexpr bool test(const size_t index) const noexcept {
            const auto [page, bit] = this->unfold_index(index);
            return ((this->data[page] & (static_cast<page_t>(1) << bit)) != 0);
        }

        /** True, if no bits are set */
        [[nodiscard]] constexpr bool empty() const noexcept {
            return std::all_of(this->data.cbegin(), this->data.cend(),
                               [](auto x) { return x == static_cast<page_t>(0);});
        }

        /** Counts number of bits that are set */
        [[nodiscard]] constexpr size_t count() const noexcept {
            return std::transform_reduce(this->data.cbegin(), this->data.cend(), static_cast<size_t>(0), std::plus{},
                                         [](auto x) { return std::popcount(x); });
        }

        /** Gets index of first set bit, or bit_size if no bits are set. */
        [[nodiscard]] constexpr size_t first_index() const noexcept {
            size_t first = 0;
            for (auto page : this->data) {
                size_t page_first = std::countr_zero(page);
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
        template<std::integral int_t = size_t>
        [[nodiscard]] std::set<int_t> to_set() const {
            std::set<int_t> output;
            for (auto x : *this) {
                output.emplace_hint(output.end(), static_cast<int_t>(x));
            }
            return output;
        }

        [[nodiscard]] constexpr bool operator==(const DynamicBitset<page_t>& rhs) const noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (size_t p = 0; p < this->page_count; ++p) {
                if (this->data[p] != rhs.data[p]) {
                    return false;
                }
            }
            return true;
        }

        [[nodiscard]] constexpr bool operator!=(const DynamicBitset<page_t>& rhs) const noexcept {
            return !this->operator==(rhs);
        }


        constexpr DynamicBitset<page_t>& operator&=(const DynamicBitset<page_t>& rhs) noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (size_t index = 0; index < this->page_count; ++index) {
                this->data[index] &= rhs.data[index];
            }
            return *this;
        }

        [[nodiscard]] constexpr DynamicBitset<page_t> operator&(const DynamicBitset<page_t>& rhs) const noexcept {
            DynamicBitset<page_t> copy{*this};
            copy &= rhs;
            return copy;
        }

        constexpr DynamicBitset<page_t>& operator|=(const DynamicBitset<page_t>& rhs) noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (size_t index = 0; index < this->page_count; ++index) {
                this->data[index] |= rhs.data[index];
            }
            return *this;
        }

        [[nodiscard]] constexpr DynamicBitset<page_t> operator|(const DynamicBitset<page_t>& rhs) const noexcept {
            DynamicBitset<page_t> copy{*this};
            copy |= rhs;
            return copy;
        }

    private:
        [[nodiscard]] constexpr std::pair<size_t, size_t> unfold_index(const size_t index) const noexcept {
            assert(index < bit_size);
            const size_t page = index / std::numeric_limits<page_t>::digits;
            const size_t bit = index % std::numeric_limits<page_t>::digits;
            assert(page < page_count);
            return {page, bit};
        }

    private:
        [[nodiscard]] constexpr static size_t pages_required(const size_t size) noexcept {
            if (0 == (size % std::numeric_limits<page_t>::digits)) {
                return size / std::numeric_limits<page_t>::digits;
            } else {
                return (size / std::numeric_limits<page_t>::digits) + 1;
            }
        }

        [[nodiscard]] constexpr static page_t make_final_mask(const size_t bit_size) noexcept {
            const auto remainder = bit_size % std::numeric_limits<page_t>::digits;
            if (0 == remainder) {
                return ~static_cast<page_t>(0); // 0xff..ff
            }
            return (static_cast<page_t>(1) << (remainder)) - 1; // e.g (1<<63)=0x80...; -1 sets all but the highest bit.
        }


    };


}