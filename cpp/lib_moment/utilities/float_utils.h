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
#include <concepts>
#include <complex>
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
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::max(std::abs(x), std::abs(y));
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
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::min(std::abs(x), std::abs(y));
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
}