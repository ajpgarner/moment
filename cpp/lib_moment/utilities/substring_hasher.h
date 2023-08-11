/**
 * substring_hasher.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "hashed_sequence.h"

#include <cassert>

#include <iterator>

namespace Moment::Algebraic {

    /**
     * For iterating over the hashes of every substring of an operator
     */
    class SubstringHashIter {
    public:
        using iterator_category = std::input_iterator_tag;
        using difference_type = ptrdiff_t;
        using value_type = uint64_t;
        struct end_tag_t {};

    private:
        const sequence_storage_t * data_ptr;
        uint64_t radix;

        ptrdiff_t _substring_start = 0;
        ptrdiff_t _substring_end = 0;
        uint64_t _stride = 1;
        uint64_t _current_hash;

    public:
        constexpr SubstringHashIter(const sequence_storage_t& ss, size_t radix)
            : data_ptr(&ss), radix{radix},
              _substring_start{static_cast<ptrdiff_t>(ss.size() - 1)},
              _substring_end{_substring_start+1} {

            if (_substring_start >= 0) [[likely]] {
                this->_current_hash = (*data_ptr)[_substring_start] + 2;
            }
        }

        constexpr SubstringHashIter(const sequence_storage_t& ss, const end_tag_t& /**/)
            : data_ptr(&ss), _substring_start{-1}, _substring_end{0}, radix{0} { }

        SubstringHashIter(sequence_storage_t&& ss) = delete;

        SubstringHashIter(sequence_storage_t&& ss, const end_tag_t&) = delete;


        [[nodiscard]] constexpr bool operator==(const SubstringHashIter& rhs) const noexcept {
            assert(this->data_ptr == rhs.data_ptr);
            if (this->_substring_start != rhs._substring_start) {
                return false;
            }
            return (this->_substring_end == rhs._substring_end);
        }

        [[nodiscard]] constexpr bool operator!=(const SubstringHashIter& rhs) const noexcept {
            return !this->operator==(rhs);
        }

        SubstringHashIter& operator++() noexcept {
            --this->_substring_start;
            if (this->_substring_start < 0) {
                --this->_substring_end;
                this->_substring_start = this->_substring_end - 1;
                this->_stride = 1;
                this->_current_hash = 1;
            } else {
                this->_stride *= this->radix;
            }

            // Not in end state...
            if (this->_substring_start >= 0) {
                this->_current_hash += (1+(*this->data_ptr)[_substring_start]) * this->_stride;
            }

            return *this;
        }

        [[deprecated("Prefer ++iter")]] const SubstringHashIter operator++(int) & noexcept {
            SubstringHashIter useless_copy{*this};
            ++(*this);
            return useless_copy;
        }

        [[nodiscard]] constexpr uint64_t operator*() const noexcept {
            assert(this->_substring_start >= 0);
            return this->_current_hash;
        }

        [[nodiscard]] constexpr size_t index() const noexcept {
            assert(this->_substring_start >= 0);
            return static_cast<size_t>(this->_substring_start);
        }

    };

    static_assert(std::input_iterator<SubstringHashIter>);

    class SubstringHashRange {
    public:
        const sequence_storage_t& sequence_string;
        const size_t radix;

        constexpr SubstringHashRange(const sequence_storage_t& ss, const size_t radix)
            : sequence_string{ss}, radix{radix} { }

        constexpr SubstringHashRange(const HashedSequence& ss, const size_t radix)
            : sequence_string{ss.raw()}, radix{radix} { }

        [[nodiscard]] constexpr auto begin() const noexcept {
            return SubstringHashIter{sequence_string, radix};
        }

        [[nodiscard]] constexpr auto end() const noexcept {
            return SubstringHashIter{sequence_string, SubstringHashIter::end_tag_t{}};
        }


    };
}