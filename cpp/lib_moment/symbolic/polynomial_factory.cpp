/**
 * polynomial_factory.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_factory.h"

#include <algorithm>
#include <iostream>
#include <numeric>


namespace Moment {
    std::ostream& operator<<(std::ostream& os, const PolynomialFactory& factory) {
        os << factory.name() << ", floating-point tolerance multiplier = " << factory.zero_tolerance << ".";
        return os;
    }


    SmallVector<size_t, 1> PolynomialFactory::presort_data(Polynomial::storage_t& data) const {
        // Fill indices
        SmallVector<size_t, 1> sort_order(data.size(), 0);
        std::iota(sort_order.begin(), sort_order.end(), 0);

        // Get sort order
        std::stable_sort(sort_order.begin(), sort_order.end(),
                         [&](const size_t lhs_index, size_t rhs_index) {
                             return this->less(data[lhs_index], data[rhs_index]);
                         });

        // Apply sort, if done
        if (!std::is_sorted(sort_order.begin(), sort_order.end())) {
            Polynomial::storage_t sorted_data;
            sorted_data.reserve(data.size());
            for (const auto idx : sort_order) {
                sorted_data.emplace_back(data[idx]);
            }
            data.swap(sorted_data);
        }
        return sort_order;
    }

    Polynomial PolynomialFactory::sum(const Monomial& lhs, const Monomial& rhs) const {
        // "Monomial"-like sum
        if ((lhs.id == rhs.id) && (lhs.conjugated == rhs.conjugated)) {
            std::complex<double> factor = lhs.factor + rhs.factor;
            if (approximately_zero(factor, this->zero_tolerance)) {
                return Polynomial::Zero();
            } else {
                return Polynomial{Monomial{lhs.id, factor, lhs.conjugated}};
            }
        }

        // Otherwise, construct as two element sum:
        if (this->less(lhs, rhs)) {
            return Polynomial{Polynomial::init_raw_tag{}, Polynomial::storage_t{lhs, rhs}};
        } else {
            return Polynomial{Polynomial::init_raw_tag{}, Polynomial::storage_t{rhs, lhs}};
        }
    }

    Polynomial PolynomialFactory::sum(const Polynomial& lhs, const Monomial& rhs) const {
        // TODO: Efficient addition assuming LHS is sorted
        Polynomial output{lhs};
        this->append(output, Polynomial(rhs, this->zero_tolerance)); // <- virtual call.
        return output;
    }

    Polynomial PolynomialFactory::sum(const Polynomial& lhs, const Polynomial& rhs) const {
        // TODO: Efficient addition assuming LHS and RHS are sorted
        Polynomial output{lhs};
        this->append(output, rhs); // <- virtual call.
        return output;
    }

}