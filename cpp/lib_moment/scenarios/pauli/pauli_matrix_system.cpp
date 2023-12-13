/**
 * pauli_matrix_system.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_matrix_system.h"
#include "pauli_context.h"
#include "scenarios/pauli/matrices/monomial_localizing_matrix.h"
#include "scenarios/pauli/matrices/polynomial_matrices.h"
#include "scenarios/pauli/matrices/moment_matrix.h"

#include "matrix/polynomial_matrix.h"

#include <cassert>

namespace Moment::Pauli {
    PauliMatrixSystem::PauliMatrixSystem(std::unique_ptr<class PauliContext> contextPtr, const double tolerance)
        : MatrixSystem{std::move(contextPtr), tolerance},
          pauliContext{dynamic_cast<class PauliContext&>(this->Context())},
          PauliMomentMatrices{*this}, PauliLocalizingMatrices{*this}, PauliPolynomialLocalizingMatrices{*this},
          CommutatorMatrices{*this}, AnticommutatorMatrices{*this},
          PolynomialCommutatorMatrices{*this}, PolynomialAnticommutatorMatrices{*this} {

        // Set polynomial factory for indices...
        this->PauliPolynomialLocalizingMatrices.indices.set_factory(this->polynomial_factory());
        this->PolynomialCommutatorMatrices.indices.set_factory(this->polynomial_factory());
        this->PolynomialAnticommutatorMatrices.indices.set_factory(this->polynomial_factory());
    }

    std::pair<ptrdiff_t, const PolynomialMatrix&>
    PauliMatrixSystem::create_and_register_localizing_matrix(const NearestNeighbourIndex& index,
                                                             const RawPolynomial& raw_poly,
                                                             Multithreading::MultiThreadPolicy mt_policy) {
        auto write_lock = this->get_write_lock();
        auto mat_ptr = Pauli::PolynomialLocalizingMatrix::create_from_raw(write_lock, *this, this->PauliLocalizingMatrices,
                                                                          index, raw_poly, mt_policy);
        const auto& matrix = *mat_ptr;
        const auto offset = this->push_back(write_lock, std::move(mat_ptr));
        return {offset, matrix};
    }

    std::pair<ptrdiff_t, const PolynomialMatrix&>
    PauliMatrixSystem::create_and_register_commutator_matrix(const NearestNeighbourIndex& index,
                                                             const RawPolynomial& raw_poly,
                                                             Multithreading::MultiThreadPolicy mt_policy) {
        auto write_lock = this->get_write_lock();
        auto mat_ptr = PolynomialCommutatorMatrix::create_from_raw(write_lock, *this, this->CommutatorMatrices,
                                                                   index, raw_poly, mt_policy);
        const auto& matrix = *mat_ptr;
        const auto offset = this->push_back(write_lock, std::move(mat_ptr));
        return {offset, matrix};
    }

    std::pair<ptrdiff_t, const PolynomialMatrix&>
    PauliMatrixSystem::create_and_register_anticommutator_matrix(const NearestNeighbourIndex& index,
                                                                 const RawPolynomial& raw_poly,
                                                                 Multithreading::MultiThreadPolicy mt_policy) {
        auto write_lock = this->get_write_lock();
        auto mat_ptr = PolynomialAnticommutatorMatrix::create_from_raw(write_lock, *this, this->AnticommutatorMatrices,
                                                                       index, raw_poly, mt_policy);
        const auto& matrix = *mat_ptr;
        const auto offset = this->push_back(write_lock, std::move(mat_ptr));
        return {offset, matrix};
    }

    size_t PauliMatrixSystem::osg_size(const NearestNeighbourIndex& index) const {
        return this->pauliContext.pauli_dictionary().WordCount(index);
    }

    std::unique_ptr<SymbolicMatrix>
    PauliMatrixSystem::create_moment_matrix(const WriteLock& lock, size_t level,
                                            Multithreading::MultiThreadPolicy mt_policy) {
        // Upcast index and call
        return this->create_nearest_neighbour_moment_matrix(lock, Pauli::MomentMatrixIndex{level, 0}, mt_policy);
    }

    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_moment_matrix(const WriteLock& write_lock,
                                                              const Pauli::MomentMatrixIndex& index,
                                                              Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = Pauli::MomentMatrix::create_matrix(this->pauliContext, symbol_table, index, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
    }

    std::unique_ptr<SymbolicMatrix>
    PauliMatrixSystem::create_localizing_matrix(const WriteLock& write_lock,
                                                const ::Moment::LocalizingMatrixIndex& lmi,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        // Upcast index, and call
        return this->create_nearest_neighbour_localizing_matrix(write_lock,
                                                                static_cast<Pauli::LocalizingMatrixIndex>(lmi),
                                                                mt_policy);
    }

    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_localizing_matrix(const WriteLock& write_lock,
                                                                  const Pauli::LocalizingMatrixIndex& lmi,
                                                                  Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = Pauli::MonomialLocalizingMatrix::create_matrix(this->pauliContext, symbol_table, lmi, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
    }

    std::unique_ptr<PolynomialMatrix>
    PauliMatrixSystem::create_polynomial_localizing_matrix(const WriteLock& write_lock,
                                                           const ::Moment::PolynomialLocalizingMatrixIndex& plmi,
                                                           Multithreading::MultiThreadPolicy mt_policy) {
        // Upcast index, and call
        return this->create_nearest_neighbour_localizing_matrix(
                write_lock,
                static_cast<Pauli::PolynomialLocalizingMatrixIndex>(plmi),
                mt_policy);
    }

    std::unique_ptr<PolynomialMatrix>
    PauliMatrixSystem::create_nearest_neighbour_localizing_matrix(const WriteLock& lock,
                                                                  const Pauli::PolynomialLocalizingMatrixIndex& index,
                                                                  Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(lock));
        return PolynomialLocalizingMatrix::create(lock, *this, this->PauliLocalizingMatrices, index, mt_policy);
    }

    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_commutator_matrix(const WriteLock& write_lock,
                                                const CommutatorMatrixIndex& cmi,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = MonomialCommutatorMatrix::create_matrix(this->pauliContext, symbol_table, cmi, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
    }

    std::unique_ptr<PolynomialMatrix>
    PauliMatrixSystem::create_commutator_matrix(const WriteLock& lock,
                                                const PolynomialCommutatorMatrixIndex& index,
                                                Multithreading::MultiThreadPolicy mt_policy) {

        assert(this->is_locked_write_lock(lock));
        return PolynomialCommutatorMatrix::create(lock, *this, this->CommutatorMatrices, index, mt_policy);
    }


    std::unique_ptr<MonomialMatrix>
    PauliMatrixSystem::create_anticommutator_matrix(const WriteLock& write_lock,
                                                    const AnticommutatorMatrixIndex& cmi,
                                                    Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(write_lock));
        auto& symbol_table = this->Symbols();
        const size_t prev_symbol_count = symbol_table.size();
        auto ptr = MonomialAnticommutatorMatrix::create_matrix(this->pauliContext, symbol_table, cmi, mt_policy);
        const size_t new_symbol_count = symbol_table.size();
        if (new_symbol_count > prev_symbol_count) {
            this->on_new_symbols_registered(write_lock, prev_symbol_count, new_symbol_count);
        }
        return ptr;
    }

    std::unique_ptr<PolynomialMatrix>
    PauliMatrixSystem::create_anticommutator_matrix(const WriteLock& lock,
                                                    const PolynomialAnticommutatorMatrixIndex& index,
                                                    Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->is_locked_write_lock(lock));
        return PolynomialAnticommutatorMatrix::create(lock, *this, this->AnticommutatorMatrices, index, mt_policy);
    }



    void PauliMatrixSystem::on_new_moment_matrix(const WriteLock& write_lock,
                                                 size_t moment_matrix_level,
                                                 ptrdiff_t matrix_offset,
                                                 const SymbolicMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add index with NN info
        Pauli::MomentMatrixIndex pmmi{moment_matrix_level, 0};
        assert(!this->PauliMomentMatrices.contains(pmmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliMomentMatrices.insert_alias(write_lock, pmmi, matrix_offset);
        assert(actual_offset == matrix_offset);

    }

    void PauliMatrixSystem::on_new_nearest_neighbour_moment_matrix(const WriteLock& write_lock,
                                                                   const Pauli::MomentMatrixIndex& index,
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

    void PauliMatrixSystem::on_new_localizing_matrix(const WriteLock& write_lock,
                                                     const ::Moment::LocalizingMatrixIndex& lmi,
                                                     ptrdiff_t matrix_offset,
                                                     const SymbolicMatrix& lm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add index with NN info
        Pauli::LocalizingMatrixIndex plmi{lmi};
        assert(!this->PauliLocalizingMatrices.contains(plmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliLocalizingMatrices.insert_alias(write_lock, std::move(plmi), matrix_offset);
        assert(actual_offset == matrix_offset);
    }

    void PauliMatrixSystem::on_new_nearest_neighbour_localizing_matrix(const WriteLock& write_lock,
                                                                       const Pauli::LocalizingMatrixIndex& plmi,
                                                                       ptrdiff_t matrix_offset,
                                                                       const MonomialMatrix& mm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add plain LM index if unrestricted
        if (0 == plmi.Index.neighbours) {
            assert(!this->LocalizingMatrix.contains(static_cast<::Moment::LocalizingMatrixIndex>(plmi)));
            [[maybe_unused]] auto actual_offset =
                    this->LocalizingMatrix.insert_alias(write_lock,
                                                        static_cast<::Moment::LocalizingMatrixIndex>(plmi),
                                                        matrix_offset);
            assert(actual_offset == matrix_offset);
        }
    }

    void PauliMatrixSystem::on_new_polynomial_localizing_matrix(const WriteLock& write_lock,
                                                                const ::Moment::PolynomialLocalizingMatrixIndex& lmi,
                                                                ptrdiff_t matrix_offset,
                                                                const PolynomialMatrix& plm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add index with NN info
        Pauli::PolynomialLocalizingMatrixIndex plmi{lmi};
        assert(!this->PauliPolynomialLocalizingMatrices.contains(plmi));
        [[maybe_unused]] auto actual_offset =
                this->PauliPolynomialLocalizingMatrices.insert_alias(write_lock, std::move(plmi), matrix_offset);
        assert(actual_offset == matrix_offset);
    }

    void PauliMatrixSystem::on_new_nearest_neighbour_localizing_matrix(const WriteLock& write_lock,
                                                                   const Pauli::PolynomialLocalizingMatrixIndex& index,
                                                                   ptrdiff_t matrix_offset,
                                                                   const PolynomialMatrix& lm) {
        assert(this->is_locked_write_lock(write_lock));

        // Add plain polynomial LM index if unrestricted
        if (0 == index.Level.neighbours) {
            assert(!this->PolynomialLocalizingMatrix.contains(
                    static_cast<::Moment::PolynomialLocalizingMatrixIndex>(index))
            );
            [[maybe_unused]] auto actual_offset =
                    this->PolynomialLocalizingMatrix.insert_alias(write_lock,
                                              static_cast<::Moment::PolynomialLocalizingMatrixIndex>(index),
                                                                  matrix_offset);
            assert(actual_offset == matrix_offset);
        }
    }

    void PauliMatrixSystem::on_new_commutator_matrix(const WriteLock& write_lock,
                                                     const CommutatorMatrixIndex& index, ptrdiff_t matrix_offset,
                                                     const MonomialMatrix& cm) {
    }

    void PauliMatrixSystem::on_new_commutator_matrix(const WriteLock& write_lock,
                                                     const PolynomialCommutatorMatrixIndex& index,
                                                     ptrdiff_t matrix_offset,
                                                     const PolynomialMatrix& cm) {
    }

    void PauliMatrixSystem::on_new_anticommutator_matrix(const WriteLock& write_lock,
                                                         const AnticommutatorMatrixIndex& index,
                                                         ptrdiff_t matrix_offset,
                                                         const MonomialMatrix& cm) {
    }

    void PauliMatrixSystem::on_new_anticommutator_matrix(const WriteLock& write_lock,
                                                         const PolynomialAnticommutatorMatrixIndex& index,
                                                         ptrdiff_t matrix_offset,
                                                         const PolynomialMatrix& cm) {
    }


    PauliMatrixSystem::~PauliMatrixSystem() = default;

}