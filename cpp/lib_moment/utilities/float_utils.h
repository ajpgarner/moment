/**
 * float_utils.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * Floating point arithmetic leads to rounding errors.
 * Thus, we should only test for approximate equality.
 */
#pragma once

#include <cmath>
#include <complex>
#include <concepts>
#include <limits>

namespace Moment {
    /**
     * Constexpr implementation of abs.
     */
    template<std::floating_point float_t>
    [[nodiscard]] constexpr inline float_t abs(float_t val) noexcept {
        return (val >= 0) ? val : -val;
    }

    /**
     * True if x is almost y. The tolerance is scaled by the larger absolute value of the numbers.
     * @param x LHS test parameter.
     * @param y RHS test parameter.
     * @param eps_multiplier
     */
    [[nodiscard]] constexpr bool approximately_equal(const double x, const double y, const double eps_multiplier = 1.0) {
        return abs(x - y)
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::max(abs(x), abs(y));
    }

    /**
     * True if |x|^2 is almost |y|^2. The tolerance is scaled by the larger absolute value of the numbers.
     * @param x LHS test parameter.
     * @param y RHS test parameter.
     * @param eps_multiplier
     */
     template<std::floating_point float_t>
    [[nodiscard]] constexpr bool approximately_same_norm(const std::complex<float_t> x,
                                                         const std::complex<float_t> y,
                                                         const float_t eps_multiplier = 1.0) {
        const float_t norm_x = std::norm(x);
        const float_t norm_y = std::norm(y);
        return abs(norm_x - norm_y)
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::max(norm_x, norm_y);
    }

    /**
     * True if x is almost zero.
     * @param x The test parameter
     * @param eps_multiplier The tolerance, in units of epsilon.
     */
    [[nodiscard]] constexpr bool approximately_zero(const double x, const double eps_multiplier = 1.0) {
        return abs(x) <= eps_multiplier * std::numeric_limits<double>::epsilon();
    }


    /**
     * True if x is almost y. The tolerance is scaled by the smaller absolute value of the numbers.
     * @param x LHS test parameter.
     * @param y RHS test parameter.
     * @param eps_multiplier
     */
    [[nodiscard]] constexpr bool essentially_equal(const double x, const double y, const double eps_multiplier = 1.0) {
        return abs(x - y)
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::min(abs(x), abs(y));
    }


    /**
     * True if x is definitely larger than y.
     * In particular, x is greater than y and x is not approximately equal to y.
     */
    [[nodiscard]] constexpr bool definitely_greater_than(const double x, const double y,
                                                         const double eps_multiplier = 1.0) {
        return (x > y) && ((x - y) > eps_multiplier*std::numeric_limits<double>::epsilon() * std::max(abs(x), abs(y)));
    }

    /**
     * True if x is definitely less than y.
     * In particular, x is less than y and x is not approximately equal to y.
     * Defined such that: !definitely_less_than(a, b) && !definitely_less_than(b, a) implies approximately_equal(a, b).
     */
    [[nodiscard]] constexpr bool definitely_less_than(const double x, const double y,
                                                      const double eps_multiplier = 1.0) {
        return (x < y) && ((y - x) > eps_multiplier*std::numeric_limits<double>::epsilon() * std::max(abs(x), abs(y)));
    }

    /** Compare x and y.
     * Returns 0 if x == y (approximately); otherwise, return -1 if x < y and +1 if x > y.
     */
    [[nodiscard]] constexpr int approximately_compare(const double x, const double y, const double eps_multiplier = 1.0) {
        if (approximately_equal(x, y, eps_multiplier)) {
            return 0;
        }
        return x < y ? -1 : +1;
    }


    /**
     * True if complex x is almost a real number (also true for zero).
     * @param x The test parameter
     * @param eps_multiplier The tolerance, in units of epsilon.
     */
    [[nodiscard]] constexpr bool approximately_real(const std::complex<double> x, const double eps_multiplier = 1.0) {
        return approximately_zero(x.imag(), eps_multiplier);
    }
    /**
     * True if complex x is almost an imaginary number (also true for zero).
     * @param x The test parameter
     * @param eps_multiplier The tolerance, in units of epsilon.
     */
    [[nodiscard]] constexpr bool approximately_imaginary(const std::complex<double> x, const double eps_multiplier = 1.0) {
        return approximately_zero(x.real(), eps_multiplier);
    }

    /**
     * True if complex x is almost zero.
     * @param x The test parameter
     * @param eps_multiplier The tolerance, in units of epsilon.
     */
    [[nodiscard]] constexpr bool approximately_zero(const std::complex<double> x, const double eps_multiplier = 1.0) {
        const double threshold = eps_multiplier * eps_multiplier
                                 * std::numeric_limits<double>::epsilon() * std::numeric_limits<double>::epsilon();
        return ((x.real() * x.real()) + (x.imag() * x.imag())) < threshold;
    }

    /**
     * True if x is almost y; equality is decided by the magnitude of their difference being close to zero.
     * @param x LHS test parameter.
     * @param y RHS test parameter.
     * @param eps_multiplier
     */
    [[nodiscard]] constexpr bool approximately_equal(const std::complex<double> x,
                                                     const std::complex<double> y,
                                                     const double eps_multiplier = 1.0) {
        return approximately_zero(x - y, eps_multiplier);
    }


    /**
     * Set complex number to be purely real (resp. purely imaginary) if very close.
     */
    template<std::floating_point float_t>
    void real_or_imaginary_if_close(std::complex<float_t>& value,
                                    const float_t eps_multiplier = static_cast<float_t>(1.0)) {
        if (approximately_zero(value.real(), eps_multiplier)) {
            value.real(0.0);
        } else if (approximately_zero(value.imag(), eps_multiplier)) {
            value.imag(0.0);
        }
    }

}