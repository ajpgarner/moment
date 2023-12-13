/**
 * pauli_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "matrix_system/matrix_indices.h"
#include "matrix_system/index_storage/map_index_storage.h"

#include "scenarios/pauli/indices/monomial_index.h"
#include "scenarios/pauli/indices/nearest_neighbour_index.h"
#include "scenarios/pauli/indices/polynomial_index.h"

#include "symbolic/polynomial.h"

namespace Moment {

    class MatrixSystem;
    class MonomialMatrix;
    class PolynomialMatrix;

    namespace Pauli {

        class PauliMatrixSystem;

        /**
         * Factory: Moment matrices restricted to nearest neighbours.
         */
        class PauliMomentMatrixFactory {
        private:
            PauliMatrixSystem& system;

        public:
            using Index = Pauli::MomentMatrixIndex;

            explicit PauliMomentMatrixFactory(MatrixSystem& system);

            explicit PauliMomentMatrixFactory(PauliMatrixSystem& system) noexcept: system{system} {}

            [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, MonomialMatrix& matrix);
        };

        /**
         * Stores moment matrices by NPA level and number of neighbours.
         */
        using PauliMomentMatrixIndices = MappedMatrixIndices<MonomialMatrix, Pauli::MomentMatrixIndex,
                PauliMomentMatrixFactory, PauliMatrixSystem>;

        /**
         * Factory: Localizing matrices restricted to nearest neighbours.
         */
        class PauliLocalizingMatrixFactory {
        private:
            PauliMatrixSystem& system;

        public:
            using Index = Pauli::LocalizingMatrixIndex;

            explicit PauliLocalizingMatrixFactory(MatrixSystem& system);

            explicit PauliLocalizingMatrixFactory(PauliMatrixSystem& system) noexcept: system{system} {}

            [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, MonomialMatrix& matrix);

        };

        /**
         * Stores monomial localizing matrices by NPA level, number of neighbours, and localizing word.
         */
        using PauliLocalizingMatrixIndices = MappedMatrixIndices<MonomialMatrix,
                                                                 Pauli::LocalizingMatrixIndex,
                                                                 PauliLocalizingMatrixFactory,
                                                                 PauliMatrixSystem>;


        /**
         * Factory: Commutator matrices (possibly restricted to nearest neighbours)
         */
        class MonomialCommutatorMatrixFactory {
        public:
            using Index = CommutatorMatrixIndex;

        private:
            PauliMatrixSystem& system;

        public:
            explicit MonomialCommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

            explicit MonomialCommutatorMatrixFactory(MatrixSystem& system);

            [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, MonomialMatrix& matrix);
        };

        /**
         * Stores monomial commutator matrices by NPA level, number of neighbours, and localizing word.
         */
        using CommutatorMatrixIndices = MappedMatrixIndices<MonomialMatrix, CommutatorMatrixIndex,
                MonomialCommutatorMatrixFactory, PauliMatrixSystem>;

        /**
         * Factory: Anti-commutator matrices (possibly restricted to nearest neighbours)
         */
        class MonomialAnticommutatorMatrixFactory {
        public:
            using Index = AnticommutatorMatrixIndex;

        private:
            PauliMatrixSystem& system;

        public:
            explicit MonomialAnticommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

            explicit MonomialAnticommutatorMatrixFactory(MatrixSystem& system);

            [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, MonomialMatrix& matrix);
        };

        /**
         * Stores monomial anti-commutator matrices by NPA level, number of neighbours, and localizing word.
         */
        using AnticommutatorMatrixIndices = MappedMatrixIndices<MonomialMatrix, AnticommutatorMatrixIndex,
                MonomialAnticommutatorMatrixFactory, PauliMatrixSystem>;

        /**
         * Factory: polynomial localizing matrices restricted to nearest neighbours.
         */
        class PolynomialLocalizingMatrixFactory {
        public:
            using Index = Pauli::PolynomialLocalizingMatrixIndex;

        private:
            PauliMatrixSystem& system;

        public:
            explicit PolynomialLocalizingMatrixFactory(PauliMatrixSystem& system) : system{system} {}

            explicit PolynomialLocalizingMatrixFactory(MatrixSystem& system);

            [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, PolynomialMatrix& matrix);
        };

        /**
         * Stores localizing matrices by NPA level, localizing phrase, and number of neighbours.
         */
        using PolynomialLocalizingMatrixIndices =
                MatrixIndices<PolynomialMatrix, Pauli::PolynomialLocalizingMatrixIndex,
                              PolynomialIndexStorageBase<NearestNeighbourIndex, Pauli::LocalizingMatrixIndex>,
                              Pauli::PolynomialLocalizingMatrixFactory, PauliMatrixSystem>;

        /**
         * Factory to make polynomial localizing matrices restricted to nearest neighbours.
         */
        class PolynomialCommutatorMatrixFactory {
        public:
            using Index = PolynomialCommutatorMatrixIndex;

        private:
            PauliMatrixSystem& system;

        public:
            explicit PolynomialCommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

            explicit PolynomialCommutatorMatrixFactory(MatrixSystem& system);

            [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, PolynomialMatrix& matrix);
        };

        /**
         * Stores commutator matrices by NPA level, number of neighbours, and commuting phrase.
         */
        using PolynomialCommutatorMatrixIndices = MatrixIndices<PolynomialMatrix, PolynomialCommutatorMatrixIndex,
                PolynomialIndexStorageBase<NearestNeighbourIndex, Pauli::CommutatorMatrixIndex>,
                PolynomialCommutatorMatrixFactory,
                PauliMatrixSystem>;

        /**
          * Factory to make polynomial localizing matrices restricted to nearest neighbours.
          */
        class PolynomialAnticommutatorMatrixFactory {
        public:
            using Index = PolynomialAnticommutatorMatrixIndex;

        private:
            PauliMatrixSystem& system;

        public:
            explicit PolynomialAnticommutatorMatrixFactory(PauliMatrixSystem& system) : system{system} {}

            explicit PolynomialAnticommutatorMatrixFactory(MatrixSystem& system);

            [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
            operator()(const MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, PolynomialMatrix& matrix);
        };

        /**
         * Stores anti-commutator matrices by NPA level, number of neighbours, and commuting phrase.
         */
        using PolynomialAnticommutatorMatrixIndices = MatrixIndices<PolynomialMatrix, PolynomialAnticommutatorMatrixIndex,
                PolynomialIndexStorageBase<NearestNeighbourIndex, Pauli::AnticommutatorMatrixIndex>,
                PolynomialAnticommutatorMatrixFactory,
                PauliMatrixSystem>;
    }
}