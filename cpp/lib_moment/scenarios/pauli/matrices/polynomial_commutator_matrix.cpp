/**
 * polynomial_commutator_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_commutator_matrix.h"

#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/pauli_context.h"

#include "dictionary/raw_polynomial.h"

#include <sstream>


namespace Moment::Pauli {
    namespace {
        [[nodiscard]] inline auto pad_c_index(const NearestNeighbourIndex& index) {
            return PolynomialCommutatorMatrixIndex{index.moment_matrix_level, index.neighbours, Polynomial::Zero()};
        }

        [[nodiscard]] inline auto pad_ac_index(const NearestNeighbourIndex& index) {
            return PolynomialAnticommutatorMatrixIndex{index.moment_matrix_level, index.neighbours, Polynomial::Zero()};
        }
    }

    PolynomialCommutatorMatrix::PolynomialCommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                                           const PolynomialFactory& factory,
                                                           Index index_in,
                                                           CompositeMatrix::ConstituentInfo&& constituents)
       : CompositeMatrix{context, symbols, factory, std::move(constituents)}, pauli_context{context},
         index{std::move(index_in)} {
        this->description = this->index.to_string(context, symbols);
    }

    PolynomialCommutatorMatrix::PolynomialCommutatorMatrix(
            PauliMatrixSystem& system, NearestNeighbourIndex index,
            const std::string& raw_word_name, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
            : CompositeMatrix{system.pauliContext, system.Symbols(),
                              system.polynomial_factory(),  std::move(constituents)},
              pauli_context{system.pauliContext}, index{pad_c_index(index)}
    {

        this->description = this->index.to_string(system.pauliContext, system.Symbols());
    }

    std::unique_ptr<PolynomialCommutatorMatrix>
    PolynomialCommutatorMatrix::create_from_raw(MaintainsMutex::WriteLock& write_lock,
                                                     PauliMatrixSystem& system, NearestNeighbourIndex index,
                                                     const RawPolynomial& raw_polynomials,
                                                     Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(write_lock));

        // First ensure constituent parts exist
        PolynomialCommutatorMatrix::ConstituentInfo constituents;
        constituents.elements.reserve(raw_polynomials.size());
        for (auto& [op_seq, factor] : raw_polynomials) {
            auto [mono_offset, mono_matrix] =
                    system.CommutatorMatrices.create(write_lock, CommutatorMatrixIndex{index, op_seq}, mt_policy);
            constituents.elements.emplace_back(&mono_matrix, factor);
        }
        if (!constituents.auto_set_dimension()) {
            constituents.matrix_dimension = system.pauliContext.pauli_dictionary().WordCount(index);
        }

        // Now, make raw matrix from this
        return std::make_unique<PolynomialCommutatorMatrix>(system, index,
                                                            raw_polynomials.to_string(system.Context()),
                                                            std::move(constituents));
    }



    PolynomialAnticommutatorMatrix::PolynomialAnticommutatorMatrix(
            PauliMatrixSystem& system, NearestNeighbourIndex nn_index,
            const std::string& raw_word_name, PolynomialLocalizingMatrix::ConstituentInfo&& constituents)
            : CompositeMatrix{system.pauliContext, system.Symbols(),
                              system.polynomial_factory(),  std::move(constituents)},
              pauli_context{system.pauliContext}, index{pad_ac_index(nn_index)}
    {
        this->description = this->index.to_string(system.Context(), system.Symbols());
    }

    PolynomialAnticommutatorMatrix::PolynomialAnticommutatorMatrix(const PauliContext& context, SymbolTable& symbols,
                                                                   const PolynomialFactory& factory,
                                                                   Index index_in,
                                                                   CompositeMatrix::ConstituentInfo&& constituents)
        : CompositeMatrix{context, symbols, factory, std::move(constituents)},
           pauli_context{context}, index{std::move(index_in)}
    {
        this->description = this->index.to_string(context, symbols);
    }

    std::unique_ptr<PolynomialAnticommutatorMatrix>
    PolynomialAnticommutatorMatrix::create_from_raw(MaintainsMutex::WriteLock& write_lock,
                                                    PauliMatrixSystem& system, NearestNeighbourIndex nn_index,
                                                    const RawPolynomial& raw_polynomials,
                                                    Multithreading::MultiThreadPolicy mt_policy) {
        assert(system.is_locked_write_lock(write_lock));

        // First ensure constituent parts exist
        PolynomialCommutatorMatrix::ConstituentInfo constituents;
        constituents.elements.reserve(raw_polynomials.size());
        for (auto& [op_seq, factor] : raw_polynomials) {
            auto [mono_offset, mono_matrix] =
                    system.AnticommutatorMatrices.create(write_lock,
                                                         AnticommutatorMatrixIndex{nn_index, op_seq},
                                                         mt_policy);
            constituents.elements.emplace_back(&mono_matrix, factor);
        }
        if (!constituents.auto_set_dimension()) {
            constituents.matrix_dimension = system.pauliContext.pauli_dictionary().WordCount(nn_index);
        }

        // Now, make raw matrix from this
        return std::make_unique<PolynomialAnticommutatorMatrix>(system, nn_index,
                                                                raw_polynomials.to_string(system.Context()),
                                                                std::move(constituents));

    }



}