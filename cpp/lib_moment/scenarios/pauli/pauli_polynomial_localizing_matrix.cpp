/**
 * pauli_polynomial_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_polynomial_localizing_matrix.h"

#include "pauli_context.h"

namespace Moment::Pauli {

    PauliPolynomialLocalizingMatrix::PauliPolynomialLocalizingMatrix(
            const PauliContext& context, SymbolTable& symbols, const PolynomialFactory& factory,
            PauliPolynomialLMIndex index, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
    : PolynomialLocalizingMatrix{context, symbols, factory,
                                 static_cast<PolynomialLMIndex>(index), std::move(constituents)},
         pauliContext{context}, nn_index{std::move(index)} {
    }
}