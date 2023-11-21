/**
 * pauli_poly_lmi.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "nearest_neighbour_index.h"
#include "pauli_localizing_matrix_indices.h"

#include "matrix_system/matrix_indices.h"
#include "matrix_system/polynomial_localizing_matrix_index.h"
#include "matrix_system/polynomial_index_storage.h"

#include "multithreading/maintains_mutex.h"

namespace Moment {
    class MatrixSystem;
    class PolynomialMatrix;

    namespace Pauli {
        /**
         * Index of of NPA Hierarchy level, nearest  neighbour info, and a Polynomial.
         */
        struct PauliPolynomialLMIndex : public PolynomialLMIndexBase<NearestNeighbourIndex, PauliLocalizingMatrixIndex> {

        public:
            /** Construct Pauli-scenario polynomial localizing matrix from nearest neighbour info. */
            PauliPolynomialLMIndex(BaseIndex base_index, class Polynomial poly) noexcept
                    : PolynomialLMIndexBase<NearestNeighbourIndex, PauliLocalizingMatrixIndex>{std::move(base_index),
                                                                                               std::move(poly)} { }

            /** Construct Pauli-scenario polynomial localizing matrix by initiating nearest neighbour info.. */
            PauliPolynomialLMIndex(const size_t level, const size_t neighbours, const bool wrap,
                                   class Polynomial poly) noexcept
                    : PolynomialLMIndexBase<NearestNeighbourIndex, PauliLocalizingMatrixIndex>{
                            NearestNeighbourIndex{level, neighbours, wrap}, std::move(poly)} { }

            /** Upcast, set nearest neighbour info to 0. */
            explicit PauliPolynomialLMIndex(PolynomialLMIndex no_neighbour_index) noexcept
                    : PolynomialLMIndexBase<NearestNeighbourIndex, PauliLocalizingMatrixIndex>{
                    NearestNeighbourIndex{no_neighbour_index.Level, 0, false},
                                          std::move(no_neighbour_index.Polynomial)} {}


            /** Downcast, ignoring nearest neighbour info. */
            explicit operator PolynomialLMIndex() const {
                return {this->Level.moment_matrix_level, this->Polynomial};
            }

        };

        /**
         * Storage of nearest neighbour polynomial indices.
         */
        using PauliPolynomialLMIndexStorage = PolynomialIndexStorageBase<NearestNeighbourIndex,
                PauliLocalizingMatrixIndex>;

        /**
         * Factory to make polynomial localizing matrices restricted to nearest neighbours.
         */
        class PauliPolynomialLMFactory {
        public:
            using Index = PauliPolynomialLMIndex;

        private:
            PauliMatrixSystem& system;

        public:
            explicit PauliPolynomialLMFactory(PauliMatrixSystem& system) : system{system} {}

            explicit PauliPolynomialLMFactory(MatrixSystem& system);

            [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
            operator()(MaintainsMutex::WriteLock& lock, const Index& index,
                       Multithreading::MultiThreadPolicy mt_policy);

            void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                        ptrdiff_t offset, PolynomialMatrix& matrix);

            [[nodiscard]] std::string not_found_msg(const Index& pmi) const;

            [[nodiscard]] std::unique_lock<std::shared_mutex> get_write_lock();
        };


        using PauliPolynomialLMIndices = MatrixIndices<PolynomialMatrix, PauliPolynomialLMIndex,
                                                       PauliPolynomialLMIndexStorage, PauliPolynomialLMFactory,
                                                       PauliMatrixSystem>;

    }
}