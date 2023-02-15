/**
 * localizing_matrix_index.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "scenarios/operator_sequence.h"

namespace Moment {

    class Context;

    struct LocalizingMatrixIndex {
    public:
        size_t Level;
        OperatorSequence Word;
        size_t WordHash;
        bool WordIsHermitian;

    public:
        LocalizingMatrixIndex(const Context& context, size_t level, OperatorSequence word);

        constexpr bool operator<(const LocalizingMatrixIndex& rhs) const noexcept {
            // Order first by Level...
            if (this->Level < rhs.Level) {
                return true;
            } else if (this->Level > rhs.Level) {
                return false;
            }

            // Then by sequence hash...
            return (this->WordHash < rhs.WordHash);
        }
    };

}