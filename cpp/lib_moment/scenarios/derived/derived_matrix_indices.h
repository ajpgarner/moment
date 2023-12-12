/**
 * derived_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_indices.h"
#include "matrix_system/vector_index_storage.h"

#include "multithreading/maintains_mutex.h"

namespace Moment::Derived {

    class DerivedMatrixSystem;

    /**
     * Factory: makes derived matrices
     */
    class DerivedMatrixFactory {
    public:
        using Index = size_t;

    private:
        DerivedMatrixSystem& system;

    public:
        explicit DerivedMatrixFactory(DerivedMatrixSystem& system) noexcept : system{system} { }

        explicit DerivedMatrixFactory(MatrixSystem& system);

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, Index src_offset, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, size_t src_offset,
                    ptrdiff_t target_offset, SymbolicMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(Index src_offset) const;
    };

    /**
     * Stores derived matrices by their source index.
     */
    using DerivedMatrixIndices = MatrixIndices<SymbolicMatrix, size_t, VectorIndexStorage,
                                               DerivedMatrixFactory, DerivedMatrixSystem>;

}