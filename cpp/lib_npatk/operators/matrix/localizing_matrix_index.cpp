/**
 * localizing_matrix_index.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "localizing_matrix_index.h"
#include "../context.h"

namespace NPATK {
    LocalizingMatrixIndex::LocalizingMatrixIndex(const Context &context, size_t level, OperatorSequence word)
        : Level{level}, Word{std::move(word)},
        WordHash{context.hash(Word)}, WordIsHermitian{Word == Word.conjugate()} {

    }

}