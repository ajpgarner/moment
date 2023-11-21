/**
 * polynomial_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "polynomial_matrix.h"
#include "matrix_system/polynomial_localizing_matrix_index.h"

#include <complex>
#include <vector>

namespace Moment {

    class MonomialMatrix;

    class PolynomialLocalizingMatrix : public PolynomialMatrix {
    public:
        using Constituents = std::vector<std::pair<const SymbolicMatrix*, std::complex<double>>>;

    private:
        PolynomialLMIndex index;
        Constituents constituents;

    public:
        /** Constructor for non-empty polynomial localizing matrix. */
        PolynomialLocalizingMatrix(const Context& context, SymbolTable& symbols, const PolynomialFactory& factory,
                                   PolynomialLMIndex index, Constituents&& constituents);

    };

}