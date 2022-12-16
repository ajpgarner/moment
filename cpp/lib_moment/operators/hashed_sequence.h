/**
 * hashable_sequence.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "shortlex_hasher.h"

#include <cassert>
#include <iosfwd>
#include <vector>

namespace Moment {

    class HashedSequence {

    public:
        using const_iter_t = std::vector<oper_name_t>::const_iterator;

    protected:
        std::vector<oper_name_t> operators{};

        bool is_zero = false;

        uint64_t the_hash = 0;

    public:
        /** Construct empty sequence (identity or zero) */
        explicit constexpr HashedSequence(const bool zero = false)
            : the_hash{is_zero ? 0U : 1U}, is_zero{zero} { }

        /** Copy constructor */
        constexpr HashedSequence(const HashedSequence& rhs) = default;

        /** Move constructor */
        constexpr HashedSequence(HashedSequence&& rhs) = default;

        /** Construct a sequence, from a list of operators and its hash  */
        constexpr HashedSequence(std::vector<oper_name_t> oper_ids, size_t hash)
                : operators{std::move(oper_ids)}, the_hash{hash}, is_zero{hash == 0} { }

        /** Construct a sequence, from a list of operators and its hash  */
        HashedSequence(std::vector<oper_name_t> oper_ids, const ShortlexHasher& hasher)
            : operators{std::move(oper_ids)}, the_hash{hasher(operators)}, is_zero{false} { }

        /**
         * Get sequence hash
         */
        [[nodiscard]] constexpr uint64_t hash() const noexcept {
            return this->the_hash;
        }

        /**
         * True if the operator sequence represents zero.
         */
        [[nodiscard]] constexpr bool zero() const noexcept {
            return this->is_zero;
        }

        /** True if this sequence is a prefix of the string defined by the supplied iterators */
        [[nodiscard]] bool matches(const_iter_t test_begin, const_iter_t test_end) const noexcept;

        /**
         * Identifies the first place this sequence occurs as a substring of the string defined by the input iterators.
         * @param iter The beginning of the string to search in
         * @param iter_end The end of the string to search in
         * @return An iterator to the location of the matched string, or iter_end if no match found.
         */
        [[nodiscard]] const_iter_t matches_anywhere(const_iter_t iter, const_iter_t iter_end) const noexcept;

        /**
         * Returns the longest suffix of this sequence that is also a prefix of rhs
         * @param rhs The sequence, whose prefixes we consider
         * @return The number of characters overlapping (if any).
         */
        [[nodiscard]] ptrdiff_t suffix_prefix_overlap(const HashedSequence& rhs) const noexcept;

        /**
         * Conjugate string, as if it were a string of Hermitian operators.
         */
        [[nodiscard]] HashedSequence conjugate(const ShortlexHasher& hasher) const;

        /** Begin iterator over operators */
        [[nodiscard]] constexpr auto begin() const noexcept  { return this->operators.cbegin(); }

        /** End iterator over operators */
        [[nodiscard]] constexpr auto end() const noexcept { return this->operators.cend(); }

        /** Begin reverse iterator over operators */
        [[nodiscard]] constexpr auto rbegin() const noexcept  { return this->operators.crbegin(); }

        /** End reverse iterator over operators */
        [[nodiscard]] constexpr auto rend() const noexcept { return this->operators.crend(); }

        /**
         * True if no operators in sequence. This can be interpreted as either the identity operator if zero()
         * returns false, or as the zero operator if zero() returns true.
         */
        [[nodiscard]] constexpr bool empty() const noexcept { return this->operators.empty(); }

        /** The length of the operator string */
        [[nodiscard]] constexpr size_t size() const noexcept { return this->operators.size(); }

        /** Operator at index N */
        [[nodiscard]] oper_name_t operator[](size_t index) const noexcept {
            assert(index < this->operators.size());
            return this->operators[index];
        }

        /** Access operator string directly */
        [[nodiscard]] constexpr const auto& raw() const noexcept { return this->operators; }

        /** Ordering by hash value (i.e. shortlex) */
        [[nodiscard]] constexpr bool operator<(const HashedSequence& rhs) const noexcept {
            return this->the_hash < rhs.the_hash;
        }

        /** Test for equality */
        [[nodiscard]] constexpr bool operator==(const HashedSequence& rhs) const noexcept {
            if (this->the_hash != rhs.the_hash) {
                return false;
            }
            if (this->is_zero != rhs.is_zero) {
                return false;
            }
            if (this->operators.size() != rhs.operators.size()) {
                return false;
            }
            for (size_t i = 0, iMax = this->operators.size(); i < iMax; ++i) {
                if (this->operators[i] != rhs.operators[i]) {
                    return false;
                }
            }
            return true;
        }

        /** Test for inequality */
        [[nodiscard]] constexpr bool operator!=(const HashedSequence& rhs) const noexcept {
            return !(*this == rhs);
        }

        /** Debug output */
        friend std::ostream& operator<<(std::ostream& os, const HashedSequence& seq);
    };

}