/**
 * equality_type.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <iosfwd>
#include <cstdint>

#include "symbol.h"

namespace NPATK {

    enum struct EqualityType : uint8_t {
        none = 0x00,
        equal = 0x01,
        negated = 0x02,
        conjugated = 0x04,
        neg_conj = 0x08
    };

    constexpr EqualityType operator&(EqualityType lhs, EqualityType rhs) {
        return static_cast<EqualityType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    constexpr void operator&=(EqualityType& lhs, EqualityType rhs) {
         lhs = static_cast<EqualityType>(static_cast<uint8_t>(lhs) & static_cast<uint8_t>(rhs));
    }

    constexpr EqualityType operator|(EqualityType lhs, EqualityType rhs) {
        return static_cast<EqualityType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    constexpr void operator|=(EqualityType& lhs, EqualityType rhs) {
        lhs = static_cast<EqualityType>(static_cast<uint8_t>(lhs) | static_cast<uint8_t>(rhs));
    }

    /**
     * Swaps equal <-> negate, and conjugated <-> neg_conjugated
     */
    constexpr EqualityType negate(EqualityType lhs) {
         return static_cast<EqualityType>(((static_cast<uint8_t>(lhs) & 0x05) << 1)
                                        + ((static_cast<uint8_t>(lhs) & 0x0a) >> 1));
    }

    /**
     * Swaps equal <-> negate, and conjugated <-> neg_conjugated
     */
    constexpr EqualityType conjugate(EqualityType lhs) {
        return static_cast<EqualityType>(((static_cast<uint8_t>(lhs) & 0x03) << 2)
                                         + ((static_cast<uint8_t>(lhs) & 0x0c) >> 2));
    }

    [[nodiscard]] constexpr EqualityType equality_type(const SymbolPair &s) {
        if (s.negated) {
            if (s.conjugated) {
                return EqualityType::neg_conj;
            }
            return EqualityType::negated;
        }
        if (s.conjugated) {
            return EqualityType::conjugated;
        }
        return EqualityType::equal;
    }

    [[nodiscard]] constexpr EqualityType compose(EqualityType lhs, EqualityType rhs) {
        EqualityType output = EqualityType::none;

        // When LHS has equality, RHS passes through as identity
        if ((lhs & EqualityType::equal) == EqualityType::equal) {
            output = rhs;
        }

        // When LHS has negation, eq <-> neg, conj <-> negconj
        if ((lhs & EqualityType::negated) == EqualityType::negated) {
            output |= negate(rhs);
        }

        // When LHS has conjugation, eq <-> conj, neg <-> negconj
        if ((lhs & EqualityType::conjugated) == EqualityType::conjugated) {
            output |= conjugate(rhs);
        }

        // When LHS has negative conjugation, eq <-> neg, conj <-> negconj
        if ((lhs & EqualityType::neg_conj) == EqualityType::neg_conj) {
            output |= conjugate(negate(rhs));
        }

        return output;
    }

    /**
     * @return first: true if real part is zero, second: true if imaginary part is zero.
     */
    constexpr std::pair<bool, bool> implies_zero(EqualityType lhs) {
        if (((lhs & (EqualityType::equal | EqualityType::negated))
                == (EqualityType::equal | EqualityType::negated))
            || ((lhs & (EqualityType::conjugated | EqualityType::neg_conj))
                == (EqualityType::conjugated | EqualityType::neg_conj))) {
            return {true, true};
        }

        bool real_is_zero = ((lhs & (EqualityType::equal | EqualityType::neg_conj))
                                == (EqualityType::equal | EqualityType::neg_conj))
                           || ((lhs & (EqualityType::negated | EqualityType::conjugated))
                                == (EqualityType::negated | EqualityType::conjugated));

        bool im_is_zero = ((lhs & (EqualityType::equal | EqualityType::conjugated))
                                == (EqualityType::equal | EqualityType::conjugated))
                          || ((lhs & (EqualityType::negated | EqualityType::neg_conj))
                                == (EqualityType::negated | EqualityType::neg_conj));

        return {real_is_zero, im_is_zero};
    }




    std::ostream& operator<<(std::ostream& os, EqualityType et);

}