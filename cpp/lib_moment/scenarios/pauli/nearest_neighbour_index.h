/**
 * nearest_neighbour_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

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

        /** True if qubit N-1 is considered as adjacent to qubit 0. */
        bool wrapped = false;

        friend auto operator<=>(const NearestNeighbourIndex& lhs, const NearestNeighbourIndex& rhs) = default;

    public:
        constexpr explicit NearestNeighbourIndex(size_t mm_level, size_t neighbours = 0, bool wrap = false)
                : moment_matrix_level{mm_level},
                  neighbours{neighbours},
                  wrapped{wrap} { }
    };
}