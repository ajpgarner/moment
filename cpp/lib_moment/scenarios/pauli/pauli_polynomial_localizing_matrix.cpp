/**
 * pauli_polynomial_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_polynomial_localizing_matrix.h"

#include "pauli_context.h"

namespace Moment::Pauli {

    namespace {
        std::string make_description(const PauliContext& context, const SymbolTable& symbols,
                                     const PauliPolynomialLMIndex& index) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

            cSS << " Pauli Localizing Matrix, Level " << index.Level.moment_matrix_level << ",";
            if (index.Level.neighbours != 0) {
                cSS << index.Level.neighbours << " Neighbour";
                if (index.Level.neighbours != 1) {
                    cSS << "s";
                }
            }
            cSS << ", Phrase " << index.Polynomial;
            return ss.str();
        }
    }


    PauliPolynomialLocalizingMatrix::PauliPolynomialLocalizingMatrix(
            const PauliContext& context, SymbolTable& symbols, const PolynomialFactory& factory,
            PauliPolynomialLMIndex index, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
    : PolynomialLocalizingMatrix{context, symbols, factory,
                                 static_cast<PolynomialLMIndex>(index), std::move(constituents)},
         pauli_context{context}, nn_index{std::move(index)} {

        if (nn_index.Level.neighbours != 0) {
            this->description = make_description(context, symbols, index);
        }
    }
}