/**
 * read_symbol_or_fail.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/symbol_expression.h"

namespace Moment::mex {

    namespace errors {
        constexpr char bad_symbol[] = "bad_symbol";
    }

        /**
         * Extracts a symbol expression from element i, j of matrix, or otherwise raise a matlab error.
         * @param engine The MATLAB engine.
         * @param matrix The matrix of string values.
         * @param index_i The row of the matrix to read.
         * @param index_j The column of hte matrix to read.
         * @return SymbolExpression found by parsing matrix element.
         */
    [[nodiscard]] SymbolExpression read_symbol_or_fail(matlab::engine::MATLABEngine& engine,
                                                       const matlab::data::StringArray& matrix,
                                                       size_t index_i, size_t index_j);

}