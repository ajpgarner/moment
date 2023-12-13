/**
 * pauli_indices.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "pauli_index_collections.h"

#include "scenarios/pauli/pauli_matrix_system.h"
#include "scenarios/pauli/indices/nearest_neighbour_index.h"
#include "scenarios/pauli/indices/monomial_index.h"
#include "scenarios/pauli/indices/polynomial_index.h"

#include "matrix/monomial_matrix.h"
#include "matrix/polynomial_matrix.h"

#include "scenarios/context.h"


namespace Moment::Pauli {
    PauliMomentMatrixFactory::PauliMomentMatrixFactory(MatrixSystem &system)
            : PauliMomentMatrixFactory{dynamic_cast<PauliMatrixSystem&>(system)} { }


    std::pair<ptrdiff_t, MonomialMatrix&>
    PauliMomentMatrixFactory::operator()(const MaintainsMutex::WriteLock &lock, const Index &index,
                                         Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_nearest_neighbour_moment_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{offset, matrix};
    }

    void PauliMomentMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                          const Index& index, ptrdiff_t offset, MonomialMatrix& matrix) {
        this->system.on_new_nearest_neighbour_moment_matrix(lock, index, offset, matrix);
    }

    PauliLocalizingMatrixFactory::PauliLocalizingMatrixFactory(MatrixSystem& system)
            : PauliLocalizingMatrixFactory{dynamic_cast<PauliMatrixSystem&>(system)} { }


    std::pair<ptrdiff_t, MonomialMatrix&>
    PauliLocalizingMatrixFactory::operator()(const MaintainsMutex::WriteLock &lock, const Index &index,
                                             Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_nearest_neighbour_localizing_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{offset, matrix};
    }

    void PauliLocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                              const Index& index, ptrdiff_t offset, MonomialMatrix& matrix) {
        this->system.on_new_nearest_neighbour_localizing_matrix(lock, index, offset, matrix);
    }

    PolynomialLocalizingMatrixFactory::PolynomialLocalizingMatrixFactory(MatrixSystem& system)
            : PolynomialLocalizingMatrixFactory{dynamic_cast<PauliMatrixSystem&>(system)}  {  }


    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialLocalizingMatrixFactory::operator()(const MaintainsMutex::WriteLock& lock,
                                         const PolynomialLocalizingMatrixFactory::Index& index,
                                         Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = this->system.create_nearest_neighbour_localizing_matrix(lock, index, mt_policy);
        PolynomialMatrix& matrixRef = *matrixPtr;
        const auto matrixIndex = system.push_back(lock, std::move(matrixPtr));
        return std::pair<ptrdiff_t, PolynomialMatrix&>{matrixIndex, matrixRef};
    }

    void PolynomialLocalizingMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                          const PolynomialLocalizingMatrixFactory::Index& index,
                                          ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_nearest_neighbour_localizing_matrix(lock, index, offset, matrix);
    }


    MonomialCommutatorMatrixFactory::MonomialCommutatorMatrixFactory(MatrixSystem& system)
            : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, MonomialMatrix&>
    MonomialCommutatorMatrixFactory::operator()(const MaintainsMutex::WriteLock& lock,
                                                const MonomialCommutatorMatrixFactory::Index& index,
                                                Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.create_commutator_matrix(lock, index, mt_policy);
        MonomialMatrix& matrixRef = *matrixPtr;
        const auto matrixIndex = system.push_back(lock, std::move(matrixPtr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{matrixIndex, matrixRef};
    }

    void MonomialCommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                 const MonomialCommutatorMatrixFactory::Index& index,
                                                 const ptrdiff_t offset,
                                                 MonomialMatrix& matrix) {
        this->system.on_new_commutator_matrix(lock, index, offset, matrix);
    }


    MonomialAnticommutatorMatrixFactory::MonomialAnticommutatorMatrixFactory(MatrixSystem& system)
            : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, MonomialMatrix&>
    MonomialAnticommutatorMatrixFactory::operator()(const MaintainsMutex::WriteLock& lock,
                                                    const MonomialAnticommutatorMatrixFactory::Index& index,
                                                    Multithreading::MultiThreadPolicy mt_policy) {
        auto matrixPtr = system.create_anticommutator_matrix(lock, index, mt_policy);
        MonomialMatrix& matrixRef = *matrixPtr;
        const auto matrixIndex = system.push_back(lock, std::move(matrixPtr));
        return std::pair<ptrdiff_t, MonomialMatrix&>{matrixIndex, matrixRef};
    }

    void MonomialAnticommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                     const MonomialAnticommutatorMatrixFactory::Index& index,
                                                     const ptrdiff_t offset, MonomialMatrix& matrix) {
        this->system.on_new_anticommutator_matrix(lock, index, offset, matrix);
    }

    PolynomialCommutatorMatrixFactory::PolynomialCommutatorMatrixFactory(MatrixSystem& system)
            : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialCommutatorMatrixFactory::operator()(const MaintainsMutex::WriteLock& lock,
                                                  const PolynomialCommutatorMatrixFactory::Index& index,
                                                  Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_commutator_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, PolynomialMatrix&>{offset, matrix};
    }

    void PolynomialCommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                   const PolynomialCommutatorMatrixFactory::Index& index,
                                                   ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_commutator_matrix(lock, index, offset, matrix);
    }

    PolynomialAnticommutatorMatrixFactory::PolynomialAnticommutatorMatrixFactory(MatrixSystem& system)
            : system{dynamic_cast<PauliMatrixSystem&>(system)} { }

    std::pair<ptrdiff_t, PolynomialMatrix&>
    PolynomialAnticommutatorMatrixFactory::operator()(const MaintainsMutex::WriteLock& lock,
                                                      const PolynomialAnticommutatorMatrixFactory::Index& index,
                                                      Multithreading::MultiThreadPolicy mt_policy) {
        assert(this->system.is_locked_write_lock(lock));

        auto pauli_matrix_ptr = this->system.create_anticommutator_matrix(lock, index, mt_policy);
        auto& matrix = *pauli_matrix_ptr;
        ptrdiff_t offset = this->system.push_back(lock, std::move(pauli_matrix_ptr));
        return std::pair<ptrdiff_t, PolynomialMatrix&>{offset, matrix};
    }

    void PolynomialAnticommutatorMatrixFactory::notify(const MaintainsMutex::WriteLock& lock,
                                                       const PolynomialAnticommutatorMatrixFactory::Index& index,
                                                       ptrdiff_t offset, PolynomialMatrix& matrix) {
        this->system.on_new_anticommutator_matrix(lock, index, offset, matrix);
    }

    static_assert(makes_matrices<PauliMomentMatrixFactory, MonomialMatrix, Pauli::MomentMatrixIndex>);
    static_assert(makes_matrices<PauliLocalizingMatrixFactory, MonomialMatrix, Pauli::LocalizingMatrixIndex>);
    static_assert(makes_matrices<MonomialCommutatorMatrixFactory, MonomialMatrix, CommutatorMatrixIndex>);
    static_assert(makes_matrices<MonomialAnticommutatorMatrixFactory, MonomialMatrix, AnticommutatorMatrixIndex>);
    static_assert(makes_matrices<Pauli::PolynomialLocalizingMatrixFactory,
                                 PolynomialMatrix, Pauli::PolynomialLocalizingMatrixIndex>);
    static_assert(makes_matrices<PolynomialCommutatorMatrixFactory,
                                 PolynomialMatrix, PolynomialCommutatorMatrixIndex>);
    static_assert(makes_matrices<PolynomialAnticommutatorMatrixFactory,
                                 PolynomialMatrix, PolynomialAnticommutatorMatrixIndex>);

}