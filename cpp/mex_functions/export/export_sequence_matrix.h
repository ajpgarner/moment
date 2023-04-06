/**
 * export_sequence_matrix.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include "scenarios/operator_sequence.h"
#include "symbolic/symbol_expression.h"
#include "utilities/square_matrix.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class Context;
    class SymbolTable;
    class MatrixProperties;
    class MatrixSystem;
    class MonomialMatrix;

    namespace Locality {
        class LocalityMatrixSystem;
        class LocalityOperatorFormatter;
    };
}

namespace Moment::mex {
    /**
     * Outputs a matrix of operator sequences, as a matlab string matrix
     * @param engine The matlab engine.
     * @param matrix The matrix of operator sequences to output.
     * @return A matlab string array.
     */
    matlab::data::Array export_sequence_matrix(matlab::engine::MATLABEngine& engine,
                                               const Context& context,
                                               const SquareMatrix<OperatorSequence>& matrix);

    /**
     * Outputs a matrix of operator sequences, as a matlab string matrix, using supplied formatter
     * @param engine The matlab engine.
     * @param matrix The matrix of operator sequences to output.
     * @return A matlab string array.
     */
    matlab::data::Array
    export_sequence_matrix(matlab::engine::MATLABEngine& engine,
                           const Locality::LocalityMatrixSystem& system,
                           const Locality::LocalityOperatorFormatter& formatter,
                           const MonomialMatrix& matrix);

    /**
     * Outputs a matrix of operator sequences, as a matlab string matrix
     * @param engine The matlab engine.
     * @param matrix The matrix object
     * @return A matlab string array.
     */
    matlab::data::Array export_sequence_matrix(matlab::engine::MATLABEngine& engine,
                                              const MatrixSystem& system,
                                              const MonomialMatrix& matrix);
}
