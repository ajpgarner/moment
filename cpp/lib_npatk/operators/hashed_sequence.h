/**
 * hashable_sequence.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"
#include "shortlex_hasher.h"

#include <cassert>
#include <vector>

namespace NPATK {

    class HashedSequence {

    public:
        using const_iter_t = std::vector<oper_name_t>::const_iterator;

    protected:
        std::vector<oper_name_t> operators{};

    public:
        const size_t hash = 0;

        /** Construct empty sequence */
        constexpr HashedSequence() = default;

        /** Copy constructor */
        constexpr HashedSequence(const HashedSequence& rhs) = default;

        /** Move constructor */
        constexpr HashedSequence(HashedSequence&& rhs) = default;

        /** Construct a sequence, from a list of operators and its hash  */
        constexpr HashedSequence(std::vector<oper_name_t> oper_ids, size_t hash)
                : operators{std::move(oper_ids)}, hash{hash} { }

        /** Construct a sequence, from a list of operators and its hash  */
        HashedSequence(std::vector<oper_name_t> oper_ids, const ShortlexHasher& hasher)
            : operators{std::move(oper_ids)}, hash{hasher(operators)} { }

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

        /** True, if the operator string is empty */
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
            return this->hash < rhs.hash;
        }

        /** Test hash */
        [[nodiscard]] constexpr bool operator==(const HashedSequence& rhs) const noexcept {
            return this->hash == rhs.hash;
        }

        /** Ordering by hash value (i.e. shortlex) */
        [[nodiscard]] constexpr bool operator>(const HashedSequence& rhs) const noexcept {
            return this->hash > rhs.hash;
        }
    };

}