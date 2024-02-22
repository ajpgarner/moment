/**
 * nearest_neighbour_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * @see pauli_osg.cpp for algorithms that exploit the neighbours parameter.
 */

#pragma once

#include "integer_types.h"

#include <compare>
#include <iosfwd>
#include <string>

namespace Moment {
    class MatrixSystem;
}

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

    /**
     * Nearest neighbour index, employed to label Pauli scenario moment matrices.
     */
    struct MomentMatrixIndex : public NearestNeighbourIndex {
    public:
        using OSGIndex = NearestNeighbourIndex;

        constexpr explicit MomentMatrixIndex(const size_t mm_level, const size_t neighbours = 0)
                : NearestNeighbourIndex{mm_level, neighbours} { }

        [[nodiscard]] std::string to_string() const;

        [[nodiscard]] inline std::string to_string(const MatrixSystem& /**/) const {
            return this->to_string();
        }
    };
}
