/**
 * hermitian_type.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

namespace Moment {

    /** Flag whether an object is Hermitian, Anti-Hermitian, or both (i.e. zero), or neither. */
    enum class HermitianType : uint8_t {
        /** Object is not equal to its conjugate transpose (or negation thereof). */
        NotHermitian = 0x00,
        /** Object is equal to its conjugate transpose. */
        Hermitian = 0x01,
        /** Object is equal to the negative of its conjugate transpose. */
        AntiHermitian = 0x02,
        /** Object is zero (and hence both Hermitian and anti-Hermitian). */
        Zero = 0x03
    };

    /** True if object is Hermitian (including special case of zero). */
    [nodiscard] inline bool is_hermitian(HermitianType ht) noexcept {
        return (static_cast<uint8_t>(ht) & 0x01) == 0x01;
    }

    /** True if object is anti-Hermitian (including special case of zero). */
    [nodiscard] inline bool is_antihermitian(HermitianType ht) noexcept  {
        return (static_cast<uint8_t>(ht) & 0x02) == 0x02;
    }

    /** True if object is zero. */
    [nodiscard] inline bool is_zero(HermitianType ht) noexcept  {
        return ht == HermitianType::Zero;
    }
}