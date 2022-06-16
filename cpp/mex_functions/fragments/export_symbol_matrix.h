/**
 * exported_symbol_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "symbolic/symbol_expression.h"
#include "operators/operator_sequence.h"
#include "square_matrix.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK {
    class Context;
    class MomentMatrix;
}

namespace NPATK::mex {

    matlab::data::Array export_symbol_matrix(matlab::engine::MATLABEngine& engine,
                                             const SquareMatrix<SymbolExpression>& matrix);

    matlab::data::Array export_sequence_matrix(matlab::engine::MATLABEngine& engine,
                                               const Context& context,
                                               const SquareMatrix<OperatorSequence>& matrix);

}
