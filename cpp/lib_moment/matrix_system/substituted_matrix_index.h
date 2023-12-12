/**
 * substituted_matrix_index.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "integer_types.h"

#include <compare>
#include <concepts>
#include <iosfwd>

namespace Moment {

    struct SubstitutedMatrixIndex {
    public:
        ptrdiff_t SourceMatrix;
        ptrdiff_t Rulebook;

        constexpr SubstitutedMatrixIndex(const ptrdiff_t matrix, const ptrdiff_t rulebook) noexcept
            : SourceMatrix{matrix}, Rulebook{rulebook} { }

        template<std::integral int_t>
        constexpr SubstitutedMatrixIndex(const int_t matrix, const int_t rulebook) noexcept
            : SourceMatrix{static_cast<ptrdiff_t>(matrix)}, Rulebook{static_cast<ptrdiff_t>(rulebook)} { }

        friend auto operator<=>(const SubstitutedMatrixIndex& lhs,
                                const SubstitutedMatrixIndex& rhs) noexcept = default;

        friend std::ostream& operator<<(std::ostream& os, SubstitutedMatrixIndex index);
    };
}