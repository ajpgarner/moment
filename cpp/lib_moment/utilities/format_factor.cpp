/**
 * format_factor.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "format_factor.h"

#include "float_utils.h"

#include <iostream>

namespace Moment {

    bool format_factor(std::ostream &os, std::complex<double> factor, bool is_scalar, bool needs_plus) {
        bool need_space = true;

        if (approximately_real(factor)) { // Purely real factor
            if (factor.real() > 0) {
                if (needs_plus) {
                    os << " + ";
                }
                if (is_scalar || !approximately_equal(factor.real(), 1.0)) {
                    os << factor.real();
                } else {
                    need_space = false;
                }
            } else {
                if (needs_plus) {
                    os << " - ";
                    if (is_scalar || !approximately_equal(factor.real(), -1.0)) {
                        os << -factor.real();
                    } else {
                        need_space = false;
                    }
                } else {
                    if (is_scalar || !approximately_equal(factor.real(), -1.0)) {
                        os << factor.real();
                    } else {
                        os << "-";
                        need_space = false;
                    }
                }
            }
        } else if (approximately_zero(factor.real())) { // Purely imaginary factory
            if (factor.imag() > 0) {
                if (needs_plus) {
                    os << " + ";
                }
                if (!approximately_equal(factor.imag(), 1.0)) {
                    os << factor.imag();
                }
                os << "i";
            } else {
                if (needs_plus) {
                    os << " - ";
                    if (!approximately_equal(factor.imag(), -1.0)) {
                        os << -factor.imag();
                    }
                    os << "i";
                } else {
                    if (!approximately_equal(factor.imag(), -1.0)) {
                        os << factor.imag();
                    } else {
                        os << "-";
                    }
                    os << "i";
                }
            }
        } else { // Complex factor
            if (needs_plus) {
                os << " + ";
            }
            os << "(" << factor.real() << " + " << factor.imag() << "i)";
        }

        return need_space;
    }
}
