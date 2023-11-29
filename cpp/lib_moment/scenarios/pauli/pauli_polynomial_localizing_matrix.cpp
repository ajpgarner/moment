/**
 * pauli_polynomial_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "pauli_polynomial_localizing_matrix.h"

#include "pauli_context.h"
#include "pauli_matrix_system.h"

#include "dictionary/raw_polynomial.h"

namespace Moment::Pauli {

    namespace {
        std::string make_description(const PauliContext& context, const SymbolTable& symbols,
                                     const PauliPolynomialLMIndex& index) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;

            cSS << "Pauli Localizing Matrix, Level " << index.Level.moment_matrix_level << ",";
            if (index.Level.neighbours != 0) {
                cSS << " " << index.Level.neighbours << " Neighbour";
                if (index.Level.neighbours != 1) {
                    cSS << "s";
                }
                cSS << ",";
            }
            cSS << " Phrase " << index.Polynomial;
            return ss.str();
        }

        std::string make_from_raw_description(const PauliContext& context, const SymbolTable& symbols,
                                              const NearestNeighbourIndex& index, const std::string& base_name) {
            std::stringstream ss;
            ContextualOS cSS{ss, context, symbols};
            cSS.format_info.show_braces = false;
            cSS.format_info.display_symbolic_as = ContextualOS::DisplayAs::Operators;
            cSS << " Pauli Localizing Matrix, Level " << index.moment_matrix_level << ",";
            if (index.neighbours != 0) {
                cSS << " " <<  index.neighbours << " Neighbour";
                if (index.neighbours != 1) {
                    cSS << "s";
                }
                cSS << ",";
            }
            cSS << " Phrase " << base_name;
            return ss.str();
        }

        [[nodiscard]] inline PolynomialLMIndex pad_base_index(const NearestNeighbourIndex& index) {
            return PolynomialLMIndex{index.moment_matrix_level, Polynomial::Zero()};
        }

        [[nodiscard]] inline PauliPolynomialLMIndex pad_nn_index(const NearestNeighbourIndex& index) {
            return PauliPolynomialLMIndex{index.moment_matrix_level, index.moment_matrix_level, Polynomial::Zero()};
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

    PauliPolynomialLocalizingMatrix::PauliPolynomialLocalizingMatrix(
            PauliMatrixSystem& system, NearestNeighbourIndex index,
            const std::string& raw_word_name, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
        : PolynomialLocalizingMatrix{system.pauliContext, system.Symbols(), system.polynomial_factory(),
                                     pad_base_index(index), std::move(constituents)},
             pauli_context{system.pauliContext}, nn_index{pad_nn_index(index)}
            {
        this->description = make_from_raw_description(system.pauliContext, system.Symbols(), index, raw_word_name);
    }

    std::unique_ptr<PauliPolynomialLocalizingMatrix>
    PauliPolynomialLocalizingMatrix::create_from_raw(MaintainsMutex::WriteLock& write_lock,
                                                     PauliMatrixSystem& system, NearestNeighbourIndex index,
                                                     const RawPolynomial& raw_polynomials,
                                                     Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(write_lock));

        // First ensure constituent parts exist
        PauliPolynomialLocalizingMatrix::ConstituentInfo constituents;
        constituents.elements.reserve(raw_polynomials.size());
        for (auto& [op_seq, factor] : raw_polynomials) {
            auto [mono_offset, mono_matrix] =
                    system.PauliLocalizingMatrices.create(write_lock,
                                                          PauliLocalizingMatrixIndex{index, op_seq},
                                                          mt_policy);
            constituents.elements.emplace_back(&mono_matrix, factor);
        }
        if (!constituents.auto_set_dimension()) {
            constituents.matrix_dimension = system.pauliContext.pauli_dictionary().WordCount(index);
        }

        // Now, make raw matrix from this
        return std::make_unique<PauliPolynomialLocalizingMatrix>(system, index,
                                                                 raw_polynomials.to_string(system.Context()),
                                                                 std::move(constituents));

    }
}