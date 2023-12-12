/**
 * derived_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_indices.h"
#include "matrix_system/index_storage/vector_index_storage.h"

#include "multithreading/maintains_mutex.h"

#include <iosfwd>

namespace Moment::Derived {

    class DerivedMatrixSystem;

    struct DerivedMatrixIndex {
        size_t SourceIndex;

        /** Allow implicit casting from size_t. */
        DerivedMatrixIndex(size_t index) : SourceIndex{index} { } // NOLINT(*-explicit-constructor)

        /** Allow implicit casting to size_t. */
        [[nodiscard]] operator ::std::size_t() const noexcept { // NOLINT(*-explicit-constructor)
            return this->SourceIndex;
        }

        friend std::ostream& operator<<(std::ostream& os, DerivedMatrixIndex dmi);
    };

    /**
     * Factory: makes derived matrices
     */
    class DerivedMatrixFactory {
    public:
        using Index = DerivedMatrixIndex;

    private:
        DerivedMatrixSystem& system;

    public:
        explicit DerivedMatrixFactory(DerivedMatrixSystem& system) noexcept : system{system} { }

        explicit DerivedMatrixFactory(MatrixSystem& system);

        [[nodiscard]] std::pair<ptrdiff_t, SymbolicMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, Index src_offset, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, Index src_offset,
                    ptrdiff_t target_offset, SymbolicMatrix& matrix);
    };

    /**
     * Stores derived matrices by their source index.
     */
    using DerivedMatrixIndices = VectorMatrixIndices<SymbolicMatrix, DerivedMatrixIndex,
                                                     DerivedMatrixFactory, DerivedMatrixSystem>;

}