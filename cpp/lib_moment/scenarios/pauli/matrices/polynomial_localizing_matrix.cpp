/**
 * pauli_polynomial_localizing_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_localizing_matrix.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_matrix_system.h"

#include "dictionary/raw_polynomial.h"

namespace Moment::Pauli {

    namespace {
        [[nodiscard]] inline PolynomialLMIndex pad_base_index(const NearestNeighbourIndex& index) {
            return PolynomialLMIndex{index.moment_matrix_level, Polynomial::Zero()};
        }

        [[nodiscard]] inline Pauli::PolynomialLocalizingMatrixIndex
        pad_nn_index(const NearestNeighbourIndex& index) {
            return Pauli::PolynomialLocalizingMatrixIndex{index, Polynomial::Zero()};
        }
    }


    PolynomialLocalizingMatrix::PolynomialLocalizingMatrix(
            const PauliContext& context, SymbolTable& symbols, const PolynomialFactory& factory,
            Index index, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
    : ::Moment::PolynomialLocalizingMatrix{context, symbols, factory,
                                 static_cast<PolynomialLMIndex>(index), std::move(constituents)},
         pauli_context{context}, nn_index{std::move(index)} {
       if (nn_index.Level.neighbours != 0) {
            this->description = this->nn_index.to_string(context, symbols);
        }
    }

    PolynomialLocalizingMatrix::PolynomialLocalizingMatrix(
            PauliMatrixSystem& system, NearestNeighbourIndex index,
            const std::string& raw_word_name, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
        : ::Moment::PolynomialLocalizingMatrix{system.pauliContext, system.Symbols(), system.polynomial_factory(),
                                     pad_base_index(index), std::move(constituents)},
             pauli_context{system.pauliContext}, nn_index{pad_nn_index(index)}
            {
        this->description = this->nn_index.to_string(context, symbols);
    }

    std::unique_ptr<PolynomialLocalizingMatrix>
    PolynomialLocalizingMatrix::create_from_raw(MaintainsMutex::WriteLock& write_lock,
                                                     PauliMatrixSystem& system, NearestNeighbourIndex index,
                                                     const RawPolynomial& raw_polynomials,
                                                     Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(write_lock));

        // First ensure constituent parts exist
        PolynomialLocalizingMatrix::ConstituentInfo constituents;
        constituents.elements.reserve(raw_polynomials.size());
        for (auto& [op_seq, factor] : raw_polynomials) {
            auto [mono_offset, mono_matrix] =
                    system.PauliLocalizingMatrices.create(write_lock,
                                                          Pauli::LocalizingMatrixIndex{index, op_seq},
                                                          mt_policy);
            constituents.elements.emplace_back(&mono_matrix, factor);
        }
        if (!constituents.auto_set_dimension()) {
            constituents.matrix_dimension = system.pauliContext.pauli_dictionary().WordCount(index);
        }

        // Now, make raw matrix from this
        return std::make_unique<PolynomialLocalizingMatrix>(system, index,
                                                            raw_polynomials.to_string(system.Context()),
                                                            std::move(constituents));

    }
}