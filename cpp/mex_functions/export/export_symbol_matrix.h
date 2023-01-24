/**
 * export_symbol_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "symbolic/symbol_expression.h"
#include "utilities/square_matrix.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    /**
     * Outputs a matrix of symbol expressions, as a matlab string matrix
     * @param engine The matlab engine.
     * @param matrix The matrix of symbols to output.
     * @return A matlab string array.
     */
    matlab::data::Array export_symbol_matrix(matlab::engine::MATLABEngine &engine,
                                             const SquareMatrix <SymbolExpression> &matrix);

}