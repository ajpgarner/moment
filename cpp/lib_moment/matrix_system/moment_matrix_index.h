/**
 * moment_matrix_index..h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "integer_types.h"

#include <concepts>
#include <iosfwd>

namespace Moment {
    /** Index to a moment matrix */
    struct MomentMatrixIndex {
    public:
        /** The NPA hierarchy level */
        size_t Level;

        /** Allow implicit casting from size_t. */
        MomentMatrixIndex(size_t index) : Level{index} { } // NOLINT(*-explicit-constructor)

        /** Allow implicit casting to size_t. */
        [[nodiscard]] operator ::std::size_t() const noexcept { // NOLINT(*-explicit-constructor)
            return this->Level;
        }

        friend std::ostream& operator<<(std::ostream& os, MomentMatrixIndex index);
    };

    static_assert(std::convertible_to<MomentMatrixIndex, size_t>);
    static_assert(std::convertible_to<size_t, MomentMatrixIndex>);
}