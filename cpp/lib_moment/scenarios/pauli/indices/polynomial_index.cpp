/**
 * polynomial_index.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_index.h"

#include "dictionary/raw_polynomial.h"
#include "scenarios/pauli/pauli_matrix_system.h"


#include <sstream>

namespace Moment::Pauli {

    namespace {
        inline void add_header(ContextualOS& cSS, const std::string& matrix_name,
                                             const NearestNeighbourIndex nn_info) {
            cSS << matrix_name << " Matrix, Level " << nn_info.moment_matrix_level << ",";
            if (nn_info.neighbours != 0) {
                cSS << " " << nn_info.neighbours << " Nearest Neighbour";
                if (nn_info.neighbours != 1) {
                    cSS << "s";
                }
                cSS << ",";
            }
        }

        [[nodiscard]] inline std::string make_description(const std::string& matrix_name,
                                                          const Context& context, const SymbolTable& symbols,
                                                          const NearestNeighbourIndex nn_info,
                                                          const Polynomial& polynomial) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
            add_header(cSS, matrix_name, nn_info);

            cSS << " Phrase " << polynomial;
            return ss.str();
        }

        [[nodiscard]] inline std::string make_description_from_raw(const std::string& matrix_name,
                                                          const Context& context, const SymbolTable& symbols,
                                                          const NearestNeighbourIndex nn_info,
                                                          const RawPolynomial& polynomial) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
            add_header(cSS, matrix_name, nn_info);

            cSS << " Phrase " << polynomial.to_string(context) << " (aliased)";
            return ss.str();
        }
    }

    std::string PolynomialLocalizingMatrixIndex::to_string(const Context& context, const SymbolTable& symbols) const {
        return make_description("Localizing", context, symbols, this->Level, this->Polynomial);
    }

    std::string PolynomialLocalizingMatrixIndex::to_string(const MatrixSystem& system) const {
        return this->to_string(system.Context(), system.Symbols());
    }

    std::string PolynomialLocalizingMatrixIndex::raw_to_string(const Context& context, const SymbolTable& symbols,
                                                               const NearestNeighbourIndex& nn_index,
                                                               const RawPolynomial& raw) {
        return make_description_from_raw("Localizing", context, symbols, nn_index, raw);
    }


    std::string PolynomialCommutatorMatrixIndex::to_string(const Context& context, const SymbolTable& symbols) const {
        return make_description("Commutator", context, symbols, this->Level, this->Polynomial);
    }

    std::string PolynomialCommutatorMatrixIndex::to_string(const MatrixSystem& system) const {
        return this->to_string(system.Context(), system.Symbols());
    }

    std::string PolynomialCommutatorMatrixIndex::raw_to_string(const Context& context, const SymbolTable& symbols,
                                                               const NearestNeighbourIndex& nn_index,
                                                               const RawPolynomial& raw) {
        return make_description_from_raw("Commutator", context, symbols, nn_index, raw);
    }

    std::string PolynomialAnticommutatorMatrixIndex::to_string(const Context& context,
                                                               const SymbolTable& symbols) const {
        return make_description("Anticommutator", context, symbols, this->Level, this->Polynomial);
    }

    std::string PolynomialAnticommutatorMatrixIndex::to_string(const MatrixSystem& system) const {
        return this->to_string(system.Context(), system.Symbols());
    }

    std::string PolynomialAnticommutatorMatrixIndex::raw_to_string(const Context& context, const SymbolTable& symbols,
                                                                   const NearestNeighbourIndex& nn_index,
                                                                   const RawPolynomial& raw) {
        return make_description_from_raw("Anticommutator", context, symbols, nn_index, raw);
    }

}