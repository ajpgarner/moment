/**
 * polynomial_ordering.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial.h"
#include "polynomial_factory.h"

namespace Moment {

    /**
     * Comparator, defines ordering first on most-significant symbol, then on second, and so-forth.
     * If one string runs out of symbols before the tie is broken, and the other does not, then it is the lower string.
     */
    class PolynomialOrdering {
    public:
        const PolynomialFactory& factory;

        explicit PolynomialOrdering(const PolynomialFactory& factory) : factory{factory} { }

        /**
         * Less than.
         */
        [[nodiscard]] bool operator()(const Polynomial& lhs, const Polynomial& rhs) const noexcept {
            ptrdiff_t lhs_idx = static_cast<ptrdiff_t>(lhs.size()) - 1;
            ptrdiff_t rhs_idx = static_cast<ptrdiff_t>(rhs.size()) - 1;

            while ((lhs_idx >= 0) && (rhs_idx >= 0)) {
                const auto& lhs_elem = lhs[lhs_idx];
                const auto& rhs_elem = rhs[rhs_idx];

                if (factory.less(lhs_elem, rhs_elem)) { // Obviously less than
                    return true;
                } else if (factory.less(rhs_elem, lhs_elem)) { // Obviously not less than
                    return false;
                } // Implies lhs_elem == rhs_elem by factory's metric.

                --lhs_idx;
                --rhs_idx;
            }

            // Strings were equal until one ran out.
            // If RHS ran out first, then LHS has more terms and so is greater, so 'LHS > RHS' -> '!(LHS < RHS)'
            // If both ran out, strings are equal in terms of element names, so 'LHS = RHS' -> '!(LHS < RHS)'.
            if (lhs_idx >= rhs_idx) {
                return false;
            } else {
                // Otherwise, RHS ran out first, and so LHS is greater.
                return true;
            }
        }
    };
}

