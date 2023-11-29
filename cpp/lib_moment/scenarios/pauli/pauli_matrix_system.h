/**
 * pauli_matrix_system.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_system.h"

#include "pauli_localizing_matrix_indices.h"
#include "pauli_moment_matrix_indices.h"
#include "pauli_polynomial_lm_indices.h"
#include "commutator_matrix.h"
#include "polynomial_commutator_matrix.h"

namespace Moment::Pauli {

    class PauliContext;
    class PauliMatrixSystem : public MatrixSystem {
    public:
        const class PauliContext& pauliContext;

        /**
         * Moment matrices whose first row might be limited to nearest neighbours, etc.
         */
        PauliMomentMatrixIndices PauliMomentMatrices;

        /**
         * Localizing matrices whose first row might be limited to nearest neighbours, etc.
         */
        PauliLocalizingMatrixIndices PauliLocalizingMatrices;

        /**
         * Polynomial localizing matrices whose first row might be limited to nearest neighbours, etc.
         */
        PauliPolynomialLMIndices PauliPolynomialLocalizingMatrices;

        /**
         * Matrices of monomial terms commuted with moment matrix.
         */
        CommutatorMatrixIndices CommutatorMatrices;

        /**
         * Matrices of polynomial terms commuted with moment matrix.
         */
        PolynomialCommutatorMatrixIndices PolynomialCommutatorMatrices;

        /**
         * Matrices of monomial terms anti-commuted with moment matrix.
         */
        AnticommutatorMatrixIndices AnticommutatorMatrices;


        /**
         * Matrices of polynomial terms anti-commuted with moment matrix.
         */
        PolynomialAnticommutatorMatrixIndices PolynomialAnticommutatorMatrices;



    public:
        /**
         * Construct a system of matrices with shared operators representing Pauli matrices.
         * @param context The Pauli  scenario.
         * * @param tolerance Floating point equivalence factor.
         */
        explicit PauliMatrixSystem(std::unique_ptr<class PauliContext> context, double tolerance = 1.0);

    protected:
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        create_moment_matrix(WriteLock& lock, size_t level, Multithreading::MultiThreadPolicy mt_policy) override;

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        create_localizing_matrix(WriteLock& lock, const LocalizingMatrixIndex& lmi,
                                 Multithreading::MultiThreadPolicy mt_policy) override;

        [[nodiscard]] std::unique_ptr<PolynomialMatrix>
        create_polynomial_localizing_matrix(WriteLock& lock, const PolynomialLMIndex& index,
                                            Multithreading::MultiThreadPolicy mt_policy) override;

        /**
         * Construct a new moment matrix, with restriction of top-row elements to N-nearest neighbours.
         */
        [[nodiscard]] std::unique_ptr<MonomialMatrix>
        create_nearest_neighbour_moment_matrix(WriteLock& lock, const PauliMomentMatrixIndex& index,
                                               Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Construct a new monomial localizing matrix, with restriction of top-row elements to N-nearest neighbours.
         */
        [[nodiscard]] std::unique_ptr<MonomialMatrix>
        create_nearest_neighbour_localizing_matrix(WriteLock& lock, const PauliLocalizingMatrixIndex& index,
                                                   Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Construct a new polynomial localizing matrix, with restriction of top-row elements to N-nearest neighbours.
         */
        [[nodiscard]] std::unique_ptr<PolynomialMatrix>
        create_nearest_neighbour_localizing_matrix(WriteLock& lock, const PauliPolynomialLMIndex& index,
                                                   Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Construct a new matrix, defined as [MM, x] for moment matrix M (maybe restricted to NN) and monomial x.
         */
        [[nodiscard]] std::unique_ptr<MonomialMatrix>
        create_commutator_matrix(WriteLock& lock, const CommutatorMatrixIndex & index,
                                 Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Construct a new matrix, defined as [MM, x] for moment matrix M (maybe restricted to NN) and monomial x.
         */
        [[nodiscard]] std::unique_ptr<PolynomialMatrix>
        create_commutator_matrix(WriteLock& lock, const PolynomialCommutatorMatrixIndex & index,
                                 Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Construct a new matrix, defined as {MM, x} for moment matrix M (maybe restricted to NN) and monomial x.
         */
        [[nodiscard]] std::unique_ptr<MonomialMatrix>
        create_anticommutator_matrix(WriteLock& lock, const CommutatorMatrixIndex& index,
                                     Multithreading::MultiThreadPolicy mt_policy);

        /**
         * Construct a new polynomial matrix, defined as {MM, x} for moment matrix M (maybe restricted to NN) and monomial x.
         */
        [[nodiscard]] std::unique_ptr<PolynomialMatrix>
        create_anticommutator_matrix(WriteLock& lock, const PolynomialCommutatorMatrixIndex& index,
                                     Multithreading::MultiThreadPolicy mt_policy);


        void on_new_moment_matrix(const MaintainsMutex::WriteLock& write_lock, size_t level,
                                  ptrdiff_t matrix_offset, const SymbolicMatrix& mm) override;

        void on_new_localizing_matrix(const WriteLock& write_lock, const LocalizingMatrixIndex& lmi,
                                      ptrdiff_t matrix_offset, const SymbolicMatrix& lm) override;

        void on_new_polynomial_localizing_matrix(const WriteLock& write_lock, const PolynomialLMIndex& lmi,
                                                 ptrdiff_t matrix_offset, const PolynomialMatrix& plm) override;

        /*
         * Virtual method, called after a nearest neighbour moment matrix is generated.
         * @param level The moment matrix level.
         * @param mm The newly generated moment matrix.
         */
        virtual void on_new_nearest_neighbour_moment_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                            const PauliMomentMatrixIndex& index,
                                                            ptrdiff_t matrix_offset,
                                                            const MonomialMatrix& mm);
        /*
         * Virtual method, called after a nearest neighbour localizing matrix is generated.
         * @param level The moment matrix level.
         * @param mm The newly generated moment matrix.
         */
        virtual void on_new_nearest_neighbour_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                const PauliLocalizingMatrixIndex& index,
                                                                ptrdiff_t matrix_offset,
                                                                const MonomialMatrix& lm);

        /*
         * Virtual method, called after a nearest neighbour polynomial localizing matrix is generated.
         * @param level The moment matrix level.
         * @param mm The newly generated moment matrix.
         */
        virtual void on_new_nearest_neighbour_localizing_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                                const PauliPolynomialLMIndex& index,
                                                                ptrdiff_t matrix_offset,
                                                                const PolynomialMatrix& lm);

        /**
         * Virtual method, called after a new monomial commutator matrix is generated.
         */
        virtual void on_new_commutator_matrix(const MaintainsMutex::WriteLock& write_lock,
                                              const CommutatorMatrixIndex& index, ptrdiff_t matrix_offset,
                                              const MonomialMatrix& cm);

        /**
         * Virtual method, called after a new polynomial commutator matrix is generated.
         */
        virtual void on_new_commutator_matrix(const MaintainsMutex::WriteLock& write_lock,
                                              const PolynomialCommutatorMatrixIndex& index, ptrdiff_t matrix_offset,
                                              const PolynomialMatrix& cm);

        /**
         * Virtual method, called after a new monomial anti-commutator matrix is generated.
         */
        virtual void on_new_anticommutator_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                  const CommutatorMatrixIndex& index, ptrdiff_t matrix_offset,
                                                  const MonomialMatrix& acm);
        /**
         * Virtual method, called after a new polynomial anti-commutator matrix is generated.
         */
        virtual void on_new_anticommutator_matrix(const MaintainsMutex::WriteLock& write_lock,
                                                  const PolynomialCommutatorMatrixIndex& index, ptrdiff_t matrix_offset,
                                                  const PolynomialMatrix& acm);


    public:
        ~PauliMatrixSystem();

        [[nodiscard]] std::string system_type_name() const final {
            return "Pauli Matrix System";
        }

        friend class PauliMomentMatrixFactory;
        friend class PauliLocalizingMatrixFactory;
        friend class PauliPolynomialLMFactory;
        friend class MonomialCommutatorMatrixFactory;
        friend class PolynomialCommutatorMatrixFactory;
        friend class MonomialAnticommutatorMatrixFactory;
        friend class PolynomialAnticommutatorMatrixFactory;
    };
}
