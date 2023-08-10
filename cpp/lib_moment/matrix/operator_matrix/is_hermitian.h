/**
 * is_hermitian.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "dictionary/operator_sequence.h"
#include "tensor/square_matrix.h"

#include <array>
#include <optional>

namespace Moment {
    struct NonHInfo {
        std::array<size_t, 2> Index;

    public:
        constexpr NonHInfo(const size_t row, const size_t col) : Index{row, col} { }
        [[nodiscard]] constexpr inline size_t row() const noexcept { return Index[0]; }
        [[nodiscard]] constexpr inline size_t col() const noexcept { return Index[1]; }

        static std::optional<NonHInfo> find_first_index(const SquareMatrix<OperatorSequence>& osm);
    };

    /**
     * Sort non-H elements, to get the element with the lowest row #, followed by the lowest column #; non-existent
     * elements go at the end.
     */
    struct NonHInfoOrdering {
    public:
        [[nodiscard]] constexpr bool operator()(const std::optional<NonHInfo>& lhs,
                                      const std::optional<NonHInfo>& rhs) const noexcept {
            // First compare for value existence
            if (!lhs.has_value()) {
                return false;
            }
            if (!rhs.has_value()) {
                return true;
            }

            // Otherwise, usual comparison
            if (lhs.value().row() < rhs.value().row()) {
                return true;
            } else if (lhs.value().row() > rhs.value().row()) {
                return false;
            }
            return lhs.value().col() < rhs.value().col();
        }
    };

}