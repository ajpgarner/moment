/**
 * ipow.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <concepts>

namespace Moment {

    /**
     * Calculates integer power via "exponentiation by squaring" method
     * @tparam int_t The integer type
     * @param base The base
     * @param exp The exponent
     * @return base^exp
     */
    template<std::integral int_t>
    constexpr int_t ipow(int_t base, int_t exp) {
        // X^0 = 1, always
        if (0 == exp) {
            return 1;
        }
        if (0 == base) {
            return 0;
        }
        if (1 == base) {
            return 1;
        }

        auto output = static_cast<int_t>(1);
        while(true) {
            if ((exp & 1) == 1) {
                output *= base;
            }
            exp = (exp >> 1);
            if (0 == exp) {
                break;
            }
            base *= base;
        }
        return output;
    }
}