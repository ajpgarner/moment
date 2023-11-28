/**
 * hashable_sequence.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "sequence_sign_type.h"

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
        uint64_t the_hash;

        sequence_storage_t operators;

        SequenceSignType sign;

        /** 'Uninitialized' constructor */
        HashedSequence() = default;

    public:
        /**
         * Construct empty sequence (identity or zero)
         * @param zero True if sequence corresponds to zero, otherwise sequence is identity.
         */
        constexpr explicit HashedSequence(const bool zero)
            : operators{}, the_hash{zero ? 0U : 1U}, sign{SequenceSignType::Positive} { }

        /** Copy constructor */
        constexpr HashedSequence(const HashedSequence& rhs) = default;

        /** Move constructor */
        constexpr HashedSequence(HashedSequence&& rhs) = default;

        /** Copy assignment */
        HashedSequence& operator=(const HashedSequence& rhs) = default;

        /** Move assignment */
        HashedSequence& operator=(HashedSequence&& rhs) = default;

        /**
         * Construct a sequence, from a list of operators and its hash.
         * @param oper_ids Sequence of operator names.
         * @param hash The calculated hash of the sequence.
         * @param is_negated True if the sequence should be interpreted with a minus sign in front of it.
         */
        HashedSequence(sequence_storage_t oper_ids, const uint64_t hash,
                       SequenceSignType sign_type = SequenceSignType::Positive)
                : operators{std::move(oper_ids)}, the_hash{hash}, sign{sign_type} { }

        /**
         * Construct a sequence, from a list of operators.
         * @tparam hasher_t The hasher type.
         * @param oper_ids Sequence of operator names.
         * @param hasher Functional that applies hash to sequence.
         */
        template<OperatorHasher hasher_t>
        HashedSequence(sequence_storage_t oper_ids, const hasher_t& hasher,
                       SequenceSignType sign_type = SequenceSignType::Positive)
            :   the_hash{hasher(static_cast<const std::span<const oper_name_t>>(oper_ids))},
                operators{std::move(oper_ids)},
                sign{sign_type} { }

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
            return this->the_hash == 0;
        }

        /**
         * True, if sequence should be interpreted with a negative sign.
         */
        [[nodiscard]] constexpr bool negated() const noexcept {
            return is_negative(this->sign);
        }

        /**
         * True, if sequence should be interpreted as multiplied by imaginary unit.
         */
        [[nodiscard]] constexpr bool imaginary() const noexcept {
            return is_imaginary(this->sign);
        }

        /**
         * Get the hashed sequence's sign
         */
        [[nodiscard]] constexpr SequenceSignType get_sign() const noexcept {
            return this->sign;
        }

        /**
         * Set the hashed sequence's negation.
         */
        constexpr void set_sign(SequenceSignType new_type) noexcept {
            this->sign = new_type;
        }

        /**
         * Set the hashed sequence's negation.
         */
        [[deprecated]] constexpr void set_sign(const bool new_negation, const bool new_imaginary = false) noexcept {
            if (new_negation) {
                this->sign = new_imaginary ? SequenceSignType::NegativeImaginary : SequenceSignType::Negative;
            } else {
                this->sign = new_imaginary ? SequenceSignType::Imaginary : SequenceSignType::Positive;
            }
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

        /** Access to operator string directly */
        [[nodiscard]] constexpr const auto& raw() const noexcept { return this->operators; }

        /** Write access to operator string directly */
        [[nodiscard]] constexpr auto& raw() noexcept { return this->operators; }

        /** Recalculate sequence's hash (only required after raw access write!) */
        template<OperatorHasher hasher_t>
        inline void rehash(const hasher_t& hasher) {
            this->the_hash = hasher(static_cast<std::span<const oper_name_t>>(this->operators));
        }

        /** Manually reset sequence's hash (only required after raw access write!), or recontextualizing sequence. */
        inline void rehash(const uint64_t hash) {
            this->the_hash = hash;
        }

        /** Set a sequence to zero */
        inline void set_to_zero() {
            this->the_hash = 0;
            this->operators.clear();
            this->sign = SequenceSignType::Positive;
        }

        /** Ordering by hash value (e.g. shortlex) */
        [[nodiscard]] constexpr bool operator<(const HashedSequence& rhs) const noexcept {
            return this->the_hash < rhs.the_hash;
        }

        /** Test for equality */
        [[nodiscard]] constexpr bool operator==(const HashedSequence& rhs) const noexcept {
            // Do sequences have same hash?
            if (this->the_hash != rhs.the_hash) {
                return false;
            }

            // Sequences are equal, but are they the same sign?
            return this->sign == rhs.sign;
        }


        /**
         * Compare two sequences for equality or negative-equality.
         * @param lhs First sequence to compare.
         * @param rhs Second sequence to compare.
         * @return +1 if sequences are identical, 0 if they are completely different, -1 if lhs = -rhs.
         */
        [[nodiscard]] static int compare_same_negation(const HashedSequence &lhs, const HashedSequence &rhs) {
            // Do sequences have same hash?
            if (lhs.the_hash != rhs.the_hash) {
                return 0;
            }

            // Sequences differ by complexity
            if (lhs.imaginary() != rhs.imaginary()) {
                return 0;
            }

            // Sequences are equal, but are they the same sign?
            return lhs.negated() == rhs.negated() ? 1 : -1;
        }

        /** Test for inequality */
        [[nodiscard]] constexpr bool operator!=(const HashedSequence& rhs) const noexcept {
            return !(*this == rhs);
        }

        /** Debug output */
        friend std::ostream& operator<<(std::ostream& os, const HashedSequence& seq);
    };

}