/**
 * symbolic_matrix_helpers.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "matrix/symbolic_matrix.h"

#include <vector>

namespace Moment {
    class SymbolTable;
    class OperatorSequence;
}

namespace Moment::Tests {
    symbol_name_t find_or_fail(const SymbolTable& symbols, const OperatorSequence& seq);

    void compare_symbol_matrices(const SymbolicMatrix::SymbolMatrixView& test,
                                 const std::vector<symbol_name_t>& reference);

}