/**
 * standard_matrix_factories.h
 *
 * Specific MatrixFactories that appear in all MatrixSystems.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_indices.h"

#include "indices/localizing_matrix_index.h"
#include "indices/moment_matrix_index.h"
#include "indices/substituted_matrix_index.h"
#include "indices/polynomial_localizing_matrix_index.h"

#include "index_storage/map_index_storage.h"
#include "index_storage/polynomial_index_storage.h"
#include "index_storage/vector_index_storage.h"

#include "multithreading/multithreading.h"
#include "multithreading/maintains_mutex.h"

#include <shared_mutex>
#include <string>
#include <utility>

namespace Moment {

    class SymbolicMatrix;
    class PolynomialMatrix;
    class MatrixSystem;

    /**
     * Factory: makes moment matrices
     */
    class MomentMatrixFactory {
    public:
        using Index = MomentMatrixIndex;

    private:
        MatrixSystem& system;

    public:
        explicit MomentMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, Index level, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, Index index, ptrdiff_t offset, SymbolicMatrix& matrix);
    };


    /**
     * Stores moment matrices by integer hierarchy depth.
     */
    using MomentMatrixIndices = VectorMatrixIndices<SymbolicMatrix, MomentMatrixIndex,
                                                    MomentMatrixFactory, MatrixSystem>;

    /**
     * Factory: makes monomial localizing matrices.
     */
    class LocalizingMatrixFactory {
    public:
        using Index = LocalizingMatrixIndex;

    private:
        MatrixSystem& system;

    public:
        explicit LocalizingMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& lmi, ptrdiff_t offset, SymbolicMatrix& matrix);
    };

    /**
     * Stores monomial localizing matrices by localizing words and integer hierarchy depth.
     */
    using LocalizingMatrixIndices = MappedMatrixIndices<SymbolicMatrix, LocalizingMatrixIndex,
            LocalizingMatrixFactory, MatrixSystem>;

    /**
      * Factory: makes polynomial localizing matrices.
      */
    class PolynomialLocalizingMatrixFactory  {
    public:
        using Index = PolynomialLMIndex;

    private:
        MatrixSystem& system;

    public:
        explicit PolynomialLocalizingMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, PolynomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, PolynomialMatrix& matrix);
    };

    /**
     * Stores polynomial localizing matrices by polynomial and integer hierarchy depth.
     */
    using PolynomialLMIndices = MatrixIndices<PolynomialMatrix, PolynomialLMIndex, PolynomialIndexStorage,
            PolynomialLocalizingMatrixFactory, MatrixSystem>;

    /**
     * Factory: makes substituted matrices.
     */
    class SubstitutedMatrixFactory {
    public:
        using Index = SubstitutedMatrixIndex;

    private:
        MatrixSystem& system;

    public:
        explicit SubstitutedMatrixFactory(MatrixSystem& system) : system{system} {}

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, SymbolicMatrix& matrix);
    };

    /**
     * Stores substituted matrices by source index and rulebook index.
     */
    using SubstitutedMatrixIndices = MappedMatrixIndices<SymbolicMatrix, SubstitutedMatrixIndex,
            SubstitutedMatrixFactory, MatrixSystem>;
}