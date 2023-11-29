/**
 * polynomial_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial_matrix.h"
#include "composite_matrix.h"
#include "matrix_system/polynomial_localizing_matrix_index.h"

#include <complex>
#include <vector>

namespace Moment {

    class MonomialMatrix;

    class PolynomialLocalizingMatrix : public CompositeMatrix {
    private:
        PolynomialLMIndex index;

    public:
        /** Constructor for non-empty polynomial localizing matrix. */
        PolynomialLocalizingMatrix(const Context& context, SymbolTable& symbols, const PolynomialFactory& factory,
                                   PolynomialLMIndex index, CompositeMatrix::ConstituentInfo&& constituents);

    };

}