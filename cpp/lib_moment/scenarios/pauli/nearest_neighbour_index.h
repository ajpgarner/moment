/**
 * nearest_neighbour_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <compare>
#include <cstddef>

namespace Moment::Pauli {

    /**
     * Partial NPA level index, restricting to nearest neighbours only.
     */
    struct NearestNeighbourIndex {
    public:
        /** NPA Hierarchy level */
        size_t moment_matrix_level = 0;

        /** Number of neighbours to consider, or 0 to include all... */
        size_t neighbours = 0;

        friend auto operator<=>(const NearestNeighbourIndex& lhs, const NearestNeighbourIndex& rhs) = default;

    public:
        constexpr explicit NearestNeighbourIndex(const size_t mm_level, const size_t neighbours = 0)
                : moment_matrix_level{mm_level}, neighbours{neighbours} { }
    };
}
