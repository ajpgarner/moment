/**
 * pauli_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_matrix_system.h"
#include "pauli_context.h"
#include "pauli_localizing_matrix.h"
#include "pauli_polynomial_localizing_matrix.h"
#include "pauli_moment_matrix.h"

#include "matrix/polynomial_matrix.h"

#include <cassert>

namespace Moment::Pauli {
    PauliMatrixSystem::PauliMatrixSystem(std::unique_ptr<class PauliContext> contextPtr, const double tolerance)
        : MatrixSystem{std::move(contextPtr), tolerance},
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())},
          PauliMomentMatrices{*this}, PauliLocalizingMatrices{*this}, PauliPolynomialLocalizingMatrices{*this} {
    }

    PauliMatrixSystem::PauliMatrixSystem(const oper_name_t qubit_count, const double tolerance)
        : MatrixSystem(std::make_unique<PauliContext>(qubit_count), tolerance),
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())},
          PauliMomentMatrices{*this}, PauliLocalizingMatrices{*this}, PauliPolynomialLocalizingMatrices{*this} {

    }

    std::unique_ptr<SymbolicMatrix>
    PauliMatrixSystem::create_moment_matrix(MaintainsMutex::WriteLock& lock, size_t level,
                                            Multithreading::MultiThreadPolicy mt_policy) {
        // Upcast index and call
        return this->create_nearest_neighbour_moment_matrix(lock, NearestNeighbourIndex{level, 0, false}, mt_policy);
    }

    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_moment_matrix(MaintainsMutex::WriteLock& write_lock,
                                                              const PauliMomentMatrixIndex& index,
                                                              Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = PauliMomentMatrix::create_matrix(this->pauliContext, symbol_table, index, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
    }

    std::unique_ptr<SymbolicMatrix>
    PauliMatrixSystem::create_localizing_matrix(MaintainsMutex::WriteLock& write_lock, const LocalizingMatrixIndex& lmi,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        // Upcast index, and call
        return this->create_nearest_neighbour_localizing_matrix(write_lock,
                                                                static_cast<PauliLocalizingMatrixIndex>(lmi),
                                                                mt_policy);
    }

    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_localizing_matrix(MaintainsMutex::WriteLock& write_lock,
                                                                  const PauliLocalizingMatrixIndex& lmi,
                                                                  Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = PauliLocalizingMatrix::create_matrix(this->pauliContext, symbol_table, lmi, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
    }

    std::unique_ptr<PolynomialMatrix>
    PauliMatrixSystem::create_polynomial_localizing_matrix(MaintainsMutex::WriteLock& write_lock,
                                                           const PolynomialLMIndex& plmi,
                                                           Multithreading::MultiThreadPolicy mt_policy) {
        // Upcast index, and call
        return this->create_nearest_neighbour_localizing_matrix(write_lock,
                                                                static_cast<PauliPolynomialLMIndex>(plmi),
                                                                mt_policy);
    }

    std::unique_ptr<PolynomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_localizing_matrix(MaintainsMutex::WriteLock& lock,
                                                                  const PauliPolynomialLMIndex& index,
                                                                  Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(lock));

        auto& symbol_table = this->Symbols();

        // First ensure constituent parts exist
        PauliPolynomialLocalizingMatrix::Constituents constituents;
        constituents.reserve(index.Polynomial.size());
        for (auto [mono_index, factor] : index.MonomialIndices(symbol_table)) {
            auto [mono_offset, mono_matrix] = this->PauliLocalizingMatrices.create(lock, mono_index, mt_policy);
            constituents.emplace_back(&mono_matrix, factor);
        }

        // NB: Previous symbol updates from constituents will have already been accounted for...
        const size_t prev_symbol_count = symbol_table.size();

        // Synthesize into polynomial matrix
        auto ptr = std::make_unique<PauliPolynomialLocalizingMatrix>(this->pauliContext, symbol_table,
                                                                      this->polynomial_factory(),
                                                                      index, std::move(constituents));

        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(lock, prev_symbol_count, new_symbol_count);
        }

        return ptr;
    }


    void PauliMatrixSystem::on_new_moment_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                 size_t moment_matrix_level,
                                                 ptrdiff_t matrix_offset,
                                                 const SymbolicMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add index with NN info
        PauliMomentMatrixIndex pmmi{moment_matrix_level, 0, false};
        assert(!this->PauliMomentMatrices.contains(pmmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliMomentMatrices.insert_alias(write_lock, pmmi, matrix_offset);
        assert(actual_offset == matrix_offset);

    }

    void PauliMatrixSystem::on_new_nearest_neighbour_moment_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                   const PauliMomentMatrixIndex& index,
                                                                   ptrdiff_t matrix_offset,
                                                                   const MonomialMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add plain MM index if unrestricted
        if (0 == index.neighbours) {
            assert(!this->MomentMatrix.contains(index.moment_matrix_level));
            [[maybe_unused]] auto actual_offset =
                    this->MomentMatrix.insert_alias(write_lock, index.moment_matrix_level, matrix_offset);
            assert(actual_offset == matrix_offset);
        }
    }

    void PauliMatrixSystem::on_new_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                     const LocalizingMatrixIndex& lmi, ptrdiff_t matrix_offset,
                                                     const SymbolicMatrix& lm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add index with NN info
        PauliLocalizingMatrixIndex plmi{lmi};
        assert(!this->PauliLocalizingMatrices.contains(plmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliLocalizingMatrices.insert_alias(write_lock, std::move(plmi), matrix_offset);
        assert(actual_offset == matrix_offset);
    }

    void PauliMatrixSystem::on_new_nearest_neighbour_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                       const PauliLocalizingMatrixIndex& plmi,
                                                                       ptrdiff_t matrix_offset,
                                                                       const MonomialMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add plain LM index if unrestricted
        if (0 == plmi.Index.neighbours) {
            assert(!this->LocalizingMatrix.contains(static_cast<LocalizingMatrixIndex>(plmi)));
            [[maybe_unused]] auto actual_offset =
                    this->LocalizingMatrix.insert_alias(write_lock,
                                                        static_cast<LocalizingMatrixIndex>(plmi),
                                                        matrix_offset);
            assert(actual_offset == matrix_offset);
        }
    }

    void PauliMatrixSystem::on_new_polynomial_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                const PolynomialLMIndex& lmi, ptrdiff_t matrix_offset,
                                                                const PolynomialMatrix& plm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add index with NN info
        PauliPolynomialLMIndex plmi{lmi};
        assert(!this->PauliPolynomialLocalizingMatrices.contains(plmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliPolynomialLocalizingMatrices.insert_alias(write_lock, std::move(plmi), matrix_offset);
        assert(actual_offset == matrix_offset);
    }

    void PauliMatrixSystem::on_new_nearest_neighbour_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                       const PauliPolynomialLMIndex& index,
                                                                       ptrdiff_t matrix_offset,
                                                                       const PolynomialMatrix& lm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add plain polynomial LM index if unrestricted
        if (0 == index.Level.neighbours) {
            assert(!this->PolynomialLocalizingMatrix.contains(static_cast<PolynomialLMIndex>(index)));
            [[maybe_unused]] auto actual_offset =
                    this->PolynomialLocalizingMatrix.insert_alias(write_lock, static_cast<PolynomialLMIndex>(index),
                                                                  matrix_offset);
            assert(actual_offset == matrix_offset);
        }
    }



    PauliMatrixSystem::~PauliMatrixSystem() = default;

}