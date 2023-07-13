/**
 * polynomial_ordering.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial.h"
#include "polynomial_factory.h"

#include <cassert>

namespace Moment {

    /**
     * Comparator, defines ordering first on most-significant symbol, then on second, and so-forth.
     * If one string runs out of symbols before the tie is broken, and the other does not, then it is the lower string.
     */
    template<typename element_less_t>
    class PolynomialOrderingBase {
    protected:
        const PolynomialFactory* factoryPtr;
        element_less_t elemCompare;

    public:
        explicit PolynomialOrderingBase(const PolynomialFactory& factory,
                                        element_less_t comparator = element_less_t{})
            : factoryPtr{&factory}, elemCompare{std::move(comparator)} { }

        explicit PolynomialOrderingBase(const PolynomialFactory* factoryPtr,
                                        element_less_t comparator = element_less_t{})
                : factoryPtr{factoryPtr}, elemCompare{std::move(comparator)} { }

        /**
         * Change (or set) the factory object used for comparison.
         */
        void set_factory(const PolynomialFactory& factory) noexcept {
            this->factoryPtr = &factory;
        }

        /**
         * Less than.
         */
        [[nodiscard]] bool operator()(const Polynomial& lhs, const Polynomial& rhs) const noexcept {
            assert(factoryPtr!= nullptr);
            ptrdiff_t lhs_idx = static_cast<ptrdiff_t>(lhs.size()) - 1;
            ptrdiff_t rhs_idx = static_cast<ptrdiff_t>(rhs.size()) - 1;

            while ((lhs_idx >= 0) && (rhs_idx >= 0)) {
                switch (elemCompare(*factoryPtr, lhs[lhs_idx], rhs[rhs_idx])) {
                    case 1:
                        return true;
                    case -1:
                        return false;
                    default:
                        break;
                }

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

    class CompareMonomialWithoutCoefficients {
    public:
        [[nodiscard]] inline int operator()(const PolynomialFactory& factory,
                                     const Monomial& lhs_elem, const Monomial& rhs_elem) const noexcept {
            if (factory.less(lhs_elem, rhs_elem)) { // Obviously less than
                return 1;
            } else if (factory.less(rhs_elem, lhs_elem)) { // Obviously not less than
                return -1;
            }
            return 0;
        }
    };

    class CompareMonomialWithCoefficients {
    public:
        [[nodiscard]] inline int operator()(const PolynomialFactory& factory,
                                     const Monomial& lhs_elem, const Monomial& rhs_elem) const noexcept {
            if (factory.less(lhs_elem, rhs_elem)) { // Obviously less than
                return 1;
            } else if (factory.less(rhs_elem, lhs_elem)) { // Obviously not less than
                return -1;
            }

            // Otherwise, compare coefficient
            int result = approximately_compare(rhs_elem.factor.real(), lhs_elem.factor.real());
            if (0 == result) {
                result = approximately_compare(rhs_elem.factor.imag(), lhs_elem.factor.imag());
            }
            return result;
        }
    };

    using PolynomialOrdering = PolynomialOrderingBase<CompareMonomialWithoutCoefficients>;
    using PolynomialOrderingWithCoefficients = PolynomialOrderingBase<CompareMonomialWithCoefficients>;
}

