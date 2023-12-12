/**
 * pauli_moment_matrix_indices.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "matrix_system/matrix_indices.h"
#include "matrix_system/index_storage/map_index_storage.h"

#include "multithreading/maintains_mutex.h"

#include "integer_types.h"
#include "nearest_neighbour_index.h"

#include <set>
#include <vector>

namespace Moment {
    class MatrixSystem;
    class MonomialMatrix;
};

namespace Moment::Pauli {
    class PauliMatrixSystem;

    using PauliMomentMatrixIndex = NearestNeighbourIndex;

    class PauliMomentMatrixFactory final {
    private:
        PauliMatrixSystem& system;

    public:
        using Index = PauliMomentMatrixIndex;

        explicit PauliMomentMatrixFactory(MatrixSystem& system);

        explicit PauliMomentMatrixFactory(PauliMatrixSystem& system) noexcept : system{system} { }

        [[nodiscard]] std::pair<ptrdiff_t, MonomialMatrix&>
        operator()(MaintainsMutex::WriteLock& lock, const Index& index, Multithreading::MultiThreadPolicy mt_policy);

        void notify(const MaintainsMutex::WriteLock& lock, const Index& index,
                    ptrdiff_t offset, MonomialMatrix& matrix);

        [[nodiscard]] std::string not_found_msg(const Index& index) const;
    };

    static_assert(makes_matrices<PauliMomentMatrixFactory, MonomialMatrix, PauliMomentMatrixIndex>);

    using PauliMomentMatrixIndices = MappedMatrixIndices<MonomialMatrix, PauliMomentMatrixIndex,
                                                         PauliMomentMatrixFactory, PauliMatrixSystem>;

}