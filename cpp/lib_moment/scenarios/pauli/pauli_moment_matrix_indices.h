/**
 * pauli_moment_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_indices.h"

#include "multithreading/maintains_mutex.h"

#include "utilities/index_tree.h"
#include "utilities/set_to_vector.h"

#include "integer_types.h"

#include <set>
#include <vector>

namespace Moment {
    class MatrixSystem;
};

namespace Moment::Pauli {
    class PauliMomentMatrix;
    class PauliMatrixSystem;

    struct PauliMomentMatrixIndex {
    public:
        /** NPA Hierarchy level */
        size_t moment_matrix_level = 0;

        /** Number of neighbours to consider, or 0 to include all... */
        size_t neighbours = 0;

        /** True if qubit N-1 is considered as adjacent to qubit 0. */
        bool wrap_neighbours = false;

        friend auto operator<=>(const PauliMomentMatrixIndex& lhs, const PauliMomentMatrixIndex& rhs) = default;

    public:
        constexpr explicit PauliMomentMatrixIndex(size_t mm_level, size_t neighbours = 0, bool wrap = false)
            : moment_matrix_level{mm_level}, neighbours{neighbours}, wrap_neighbours{wrap} { }
    };

    class PauliMomentMatrixFactory final {
    private:
        PauliMatrixSystem& system;

    public:
        using Index = PauliMomentMatrixIndex;

        explicit PauliMomentMatrixFactory(MatrixSystem& system);

        explicit PauliMomentMatrixFactory(PauliMatrixSystem& system) noexcept : system{system} { }

        [[nodiscard]] std::pair<ptrdiff_t, PauliMomentMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, PauliMomentMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& index) const;

        MaintainsMutex::WriteLock get_write_lock();
    };

    static_assert(makes_matrices<PauliMomentMatrixFactory, PauliMomentMatrix, PauliMomentMatrixIndex>);

    using PauliMomentMatrixIndices = MappedMatrixIndices<PauliMomentMatrix, PauliMomentMatrixIndex,
                                                         PauliMomentMatrixFactory, PauliMatrixSystem>;

}