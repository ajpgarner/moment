/**
 * read_raw_symbol_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once


#include "utilities/square_matrix.h"
#include "symbolic/symbol_expression.h"
#include "integer_types.h"

#include "MatlabDataArray.hpp"

#include <memory>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    read_raw_symbol_matrix(matlab::engine::MATLABEngine &matlabEngine,
                           const matlab::data::Array& input);

}