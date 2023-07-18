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

}