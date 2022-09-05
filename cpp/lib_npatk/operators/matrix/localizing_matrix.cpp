/**
 * localizing_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "localizing_matrix.h"

namespace NPATK {

    LocalizingMatrix::LocalizingMatrix(const Context& context, SymbolTable& symbols, const OperatorSequence &&seedWord)
        : OperatorMatrix(context, symbols), word{seedWord}
        {

    }
}