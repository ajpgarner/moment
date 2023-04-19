/**
 * float_utils.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * Floating point arithmetic leads to rounding errors.
 * Thus, we should only test for approximate equality.
 */

#include <cmath>

#include <limits>

namespace Moment {
    /**
     * True if x is almost y. The tolerance is scaled by the larger absolute value of the numbers.
     * @param x LHS test parameter.
     * @param y RHS test parameter.
     * @param eps_multiplier
     */
    [[nodiscard]] constexpr bool approximately_equal(const double x, const double y, const double eps_multiplier = 1.0) {
        return std::abs(x - y)
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::max(std::abs(x), std::abs(y));
    }

    /**
     * True if x is almost zero.
     * @param x The test parameter
     * @param eps_multiplier The tolerance, in units of epsilon.
     */
    [[nodiscard]] constexpr bool approximately_zero(const double x, const double eps_multiplier = 1.0) {
        return std::abs(x) <= eps_multiplier * std::numeric_limits<double>::epsilon();
    }

    /**
     * True if x is almost y. The tolerance is scaled by the smaller absolute value of the numbers.
     * @param x LHS test parameter.
     * @param y RHS test parameter.
     * @param eps_multiplier
     */
    [[nodiscard]] constexpr bool essentially_equal(const double x, const double y, const double eps_multiplier = 1.0) {
        return std::abs(x - y)
               <= eps_multiplier * std::numeric_limits<double>::epsilon() * std::min(std::abs(x), std::abs(y));
    }

}