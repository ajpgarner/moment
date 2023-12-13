/**
 * polynomial_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_index.h"

#include "scenarios/pauli/pauli_matrix_system.h"


#include <sstream>

namespace Moment::Pauli {

    namespace {
        [[nodiscard]] inline std::string make_description(const std::string& matrix_name,
                                                          const Context& context, const SymbolTable& symbols,
                                                          const NearestNeighbourIndex nn_info,
                                                          const Polynomial& polynomial) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

            cSS << matrix_name << " Matrix, Level " << nn_info.moment_matrix_level << ",";
            if (nn_info.neighbours != 0) {
                cSS << " " << nn_info.neighbours << " Nearest Neighbour";
                if (nn_info.neighbours != 1) {
                    cSS << "s";
                }
                cSS << ",";
            }
            cSS << " Phrase " << polynomial;
            return ss.str();
        }
    }

    std::string PolynomialLocalizingMatrixIndex::to_string(const Context& context, const SymbolTable& symbols) const {
        return make_description("Localizing", context, symbols, this->Level, this->Polynomial);
    }

    std::string PolynomialLocalizingMatrixIndex::to_string(const MatrixSystem& system) const {
        return this->to_string(system.Context(), system.Symbols());
    }


    std::string PolynomialCommutatorMatrixIndex::to_string(const Context& context, const SymbolTable& symbols) const {
        return make_description("Commutator", context, symbols, this->Level, this->Polynomial);
    }

    std::string PolynomialCommutatorMatrixIndex::to_string(const MatrixSystem& system) const {
        return this->to_string(system.Context(), system.Symbols());
    }

    std::string PolynomialAnticommutatorMatrixIndex::to_string(const Context& context,
                                                               const SymbolTable& symbols) const {
        return make_description("Anticommutator", context, symbols, this->Level, this->Polynomial);
    }

    std::string PolynomialAnticommutatorMatrixIndex::to_string(const MatrixSystem& system) const {
        return this->to_string(system.Context(), system.Symbols());
    }

}