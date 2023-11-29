/**
 * pauli_polynomial_localizing_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix/polynomial_localizing_matrix.h"
#include "pauli_polynomial_lm_indices.h"

namespace Moment::Pauli {
    class PauliContext;

    class PauliPolynomialLocalizingMatrix : public PolynomialLocalizingMatrix {
    public:
        const PauliContext& pauli_context;
        PauliPolynomialLMIndex nn_index;

    public:
        PauliPolynomialLocalizingMatrix(const PauliContext& context, SymbolTable& symbols,
                                        const PolynomialFactory& factory,
                                        PauliPolynomialLMIndex index,
                                        PolynomialLocalizingMatrix::ConstituentInfo&& constituents);
    };
}