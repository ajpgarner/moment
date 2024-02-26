/**
 * read_symbol_or_fail.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/monomial.h"

namespace Moment::mex {

        /**
         * Extracts a symbol expression from element i, j of matrix, or otherwise raise a matlab error.
         * @param matrix The matrix of string values.
         * @param row_index The row of the matrix to read.
         * @param col_index The column of hte matrix to read.
         * @return Monomial found by parsing matrix element.
         */
    [[nodiscard]] Monomial read_symbol_or_fail(const matlab::data::StringArray& matrix,
                                               size_t row_index, size_t col_index);

        /**
         * Extracts a symbol expression from a string, or otherwise raise a matlab error.
         * @param string The string to interpret as a symbol.
         * @return Monomial found by parsing matrix element.
         */
    [[nodiscard]] Monomial read_symbol_or_fail(const matlab::data::MATLABString& string);

}