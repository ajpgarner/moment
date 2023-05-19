/**
 * symbolic_matrix_helpers.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"
#include "matrix/monomial_matrix.h"

#include <vector>


namespace Moment {
    class SymbolTable;
    class PolynomialMatrix;
    class OperatorSequence;
}

namespace Moment::Tests {
    symbol_name_t find_or_fail(const SymbolTable& symbols, const OperatorSequence& seq);

    void compare_symbol_matrices(const Matrix& test,
                                 const Matrix& reference);

    void compare_symbol_matrices(const MonomialMatrix& test,
                                 const MonomialMatrix& reference);

    void compare_symbol_matrices(const PolynomialMatrix& test,
                                 const PolynomialMatrix& reference);

    void compare_symbol_matrices(const Matrix& test,
                                 const std::vector<symbol_name_t>& reference);

    void compare_symbol_matrices(const MonomialMatrix::MMSymbolMatrixView& test,
                                 const std::vector<symbol_name_t>& reference);

}