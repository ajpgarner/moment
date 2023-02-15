/**
 * localizing_matrix_index.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "localizing_matrix_index.h"

#include "scenarios/context.h"

namespace Moment {
    LocalizingMatrixIndex::LocalizingMatrixIndex(const Context &context, size_t level, OperatorSequence word)
        : Level{level}, Word{std::move(word)},
        WordHash{context.hash(Word)}, WordIsHermitian{Word == Word.conjugate()} {
    }
}