/**
 * polynomial_factory.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_factory.h"

#include <iostream>

namespace Moment {

    std::ostream& operator<<(std::ostream& os, const PolynomialFactory& factory) {
        os << factory.name() << ", floating-point tolerance multiplier = " << factory.zero_tolerance << ".";
        return os;
    }

}