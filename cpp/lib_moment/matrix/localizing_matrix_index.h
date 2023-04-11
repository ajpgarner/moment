/**
 * localizing_matrix_index.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "scenarios/operator_sequence.h"

namespace Moment {

    struct LocalizingMatrixIndex {
    public:
        size_t Level;
        OperatorSequence Word;
        size_t WordHash;
        bool WordIsHermitian;

    public:
        LocalizingMatrixIndex(size_t level, OperatorSequence word)
                : Level{level}, Word{std::move(word)},
                  WordHash{Word.hash()}, WordIsHermitian{Word == Word.conjugate()} {
        }

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