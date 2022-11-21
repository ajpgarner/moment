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
#include <vector>

namespace NPATK {

    template<std::unsigned_integral page_t = uint64_t>
    class DynamicBitset {
    public:
        const size_t bit_size;
        const size_t page_count;

    private:
        const page_t final_page_mask;
        std::vector<page_t> data;

    public:
        constexpr explicit DynamicBitset(size_t bit_size)
            : bit_size{bit_size}, page_count{pages_required(bit_size)},
              final_page_mask{make_final_mask(bit_size)}, data(page_count, static_cast<page_t>(0)) {
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

        constexpr DynamicBitset<page_t>& operator&=(const DynamicBitset<page_t>& rhs) noexcept {
            assert(this->bit_size == rhs.bit_size);
            for (size_t index = 0; index < this->page_count; ++index) {
                this->data[index] &= rhs.data[index];
            }
            return *this;
        }

        constexpr DynamicBitset<page_t> operator&(const DynamicBitset<page_t>& rhs) const noexcept {
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

        constexpr DynamicBitset<page_t> operator|(const DynamicBitset<page_t>& rhs) const noexcept {
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