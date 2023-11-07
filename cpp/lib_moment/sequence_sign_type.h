/**
 * sequence_sign_type.h
 *
 * A representation of Z_4.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <cstdint>

#include <iosfwd>

namespace Moment {

    /**
     * The sign type of an operator sequence.
     */
    enum class SequenceSignType : uint8_t {
        /** Sequence is positive real; +1. */
        Positive = 0x00,
        /** Sequence is positive imaginary; +i. */
        Imaginary = 0x01,
        /** Sequence is negative real; -1. */
        Negative = 0x02,
        /** Sequence is negative imaginary; -i. */
        NegativeImaginary = 0x03
    };


    /**
     * Output human-readable version
     * @param os
     * @return
     */
    std::ostream& operator<<(std::ostream& os, const SequenceSignType sst);

    /**
     * Negate (1 <-> -1; i <-> -i)
     */
    [[nodiscard]] constexpr SequenceSignType negate(const SequenceSignType lhs) {
        return static_cast<SequenceSignType>(static_cast<uint64_t>(lhs) ^ 0x02);
    }

    /**
     * True if sign is negative number
     */
    [[nodiscard]] constexpr bool is_negative(const SequenceSignType lhs) {
        return static_cast<uint8_t>(lhs) & 0x02;
    }

    /**
     * True if sign is imaginary number
     */
    [[nodiscard]] constexpr bool is_imaginary(const SequenceSignType lhs) {
        return static_cast<uint8_t>(lhs) & 0x01;
    }

    /**
     * Conjugate (i <-> -i)
     */
    [[nodiscard]] constexpr SequenceSignType conjugate(const SequenceSignType lhs) {
        // 0<->0, 1<->3, 2<->2
        return static_cast<SequenceSignType>((4 - static_cast<uint8_t>(lhs)) & 0x03);
    }


    /**
     * Multiply two sign-types together.
     */
    [[nodiscard]] constexpr SequenceSignType operator*(SequenceSignType lhs, SequenceSignType rhs) {
        // This works because multiplication table of (1,i,-1,-i) is Z_4 (i.e. integer addition modulo 4).
        return static_cast<SequenceSignType>((static_cast<uint8_t>(lhs) + static_cast<uint8_t>(rhs)) & 0x03);
    }

    /**
     * The relative sign between LHS and RHS.
     * Equivalent to conjugate(LHS) * RHS.
     */
    [[nodiscard]] constexpr SequenceSignType difference(SequenceSignType lhs, SequenceSignType rhs) {
        return static_cast<SequenceSignType>((4 + static_cast<uint8_t>(rhs) - static_cast<uint8_t>(lhs)) & 0x03);
    }

    /**
     * True if two sequences are negatives of each other
     */
    [[nodiscard]] constexpr bool are_negatives(const SequenceSignType lhs, const SequenceSignType rhs) {
        return ((4 - static_cast<uint8_t>(lhs) + static_cast<uint8_t>(rhs)) & 0x03) == 0x02;
    }


}