/**
 * hashable_sequence.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "utilities/small_vector.h"

#include <cassert>

#include <concepts>
#include <iosfwd>
#include <vector>

namespace Moment {
    /** Operator string storage type. */
    using sequence_storage_t = SmallVector<oper_name_t, op_seq_stack_length>;

    /**
     * Concept: 'hasher' classes that can provide a hash to an operator sequence.
     */
    template<class hasher_class_t>
    concept OperatorHasher = requires(const hasher_class_t& hasher, const sequence_storage_t& seq) {
        {
            hasher(seq)
        } -> std::convertible_to<uint64_t>;
    };

    /**
     * Sequence of operators, and associated hash.
     */
    class HashedSequence {

    public:
        using const_iter_t = sequence_storage_t::const_iterator;

    protected:
        sequence_storage_t operators{};

        bool is_zero = false;

        uint64_t the_hash = 0;

    public:
        /**
         * Construct empty sequence (identity or zero)
         * @param zero True if sequence corresponds to zero, otherwise sequence is identity.
         */
        constexpr explicit HashedSequence(const bool zero = false)
            : the_hash{is_zero ? 0U : 1U}, is_zero{zero} { }

        /** Copy constructor */
        constexpr HashedSequence(const HashedSequence& rhs) = default;

        /** Move constructor */
        constexpr HashedSequence(HashedSequence&& rhs) = default;

        /**
         * Construct a sequence, from a list of operators and its hash.
         * @param oper_ids Sequence of operator names.
         * @param hash The calculated hash of the sequence.
         */
        HashedSequence(sequence_storage_t oper_ids, uint64_t hash)
                : operators{std::move(oper_ids)}, the_hash{hash}, is_zero{hash == 0} { }

        /**
         * Construct a sequence, from a list of operators.
         * @tparam hasher_t The hasher type.
         * @param oper_ids Sequence of operator names.
         * @param hasher Functional that applies hash to sequence.
         */
        template<OperatorHasher hasher_t>
        HashedSequence(sequence_storage_t oper_ids, const hasher_t& hasher)
            : operators{std::move(oper_ids)},
              the_hash{hasher(static_cast<std::span<const oper_name_t>>(operators))},
              is_zero{false} { }

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

        /** Begin iterator over operators */
        [[nodiscard]] constexpr auto begin() const noexcept  { return this->operators.cbegin(); }

        /** End iterator over operators */
        [[nodiscard]] constexpr auto end() const noexcept { return this->operators.cend(); }

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

        /** Ordering by hash value (e.g. shortlex) */
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