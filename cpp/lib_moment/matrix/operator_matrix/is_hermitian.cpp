/**
 * is_hermitian.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "is_hermitian.h"

namespace Moment {
    std::optional<NonHInfo> NonHInfo::find_first_index(const SquareMatrix<OperatorSequence>& osm) {
        for (size_t col = 0; col < osm.dimension; ++col) {
            auto& diag_elem = osm(std::array<size_t,2>{col, col});

            if (diag_elem != diag_elem.conjugate()) {
                return NonHInfo{col, col};
            }

            for (size_t row = col+1; row < osm.dimension; ++row) {
                const auto& upper = osm(std::array<size_t, 2>{row, col});
                const auto& lower = osm(std::array<size_t, 2>{col, row});
                const auto lower_conj = lower.conjugate();
                if (upper != lower_conj) {
                    return NonHInfo{row, col};
                }
            }
        }
        return std::nullopt;
    }
}