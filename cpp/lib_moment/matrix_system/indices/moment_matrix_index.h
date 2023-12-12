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
#include <string>

namespace Moment {
    class Context;
    class SymbolTable;
    class MatrixSystem;

    /** Index of a moment matrix (i.e. its NPA hierarchy level). */
    struct MomentMatrixIndex {
    public:
        /** The NPA hierarchy level */
        size_t Level;

        /** Allow implicit casting from size_t. */
        constexpr MomentMatrixIndex(size_t index) : Level{index} { } // NOLINT(*-explicit-constructor)

        /** Allow implicit casting to size_t. */
        [[nodiscard]] constexpr operator ::std::size_t() const noexcept { // NOLINT(*-explicit-constructor)
            return this->Level;
        }

        friend std::ostream& operator<<(std::ostream& os, MomentMatrixIndex mmi);

        [[nodiscard]] std::string to_string() const;

        [[nodiscard]] std::string to_string(const MatrixSystem& matrix_system) const;

    };

    static_assert(std::convertible_to<MomentMatrixIndex, size_t>);
    static_assert(std::convertible_to<size_t, MomentMatrixIndex>);
}