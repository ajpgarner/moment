/**
 * exported_symbol_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
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
    class SymbolicMatrix;
}

namespace Moment::mex {

    /**
     * Outputs a matrix of symbol expressions, as a matlab string matrix
     * @param engine The matlab engine.
     * @param matrix The matrix of symbols to output.
     * @return A matlab string array.
     */
    matlab::data::Array export_symbol_matrix(matlab::engine::MATLABEngine& engine,
                                             const SquareMatrix<SymbolExpression>& matrix);

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
     * Outputs a matrix of operator sequences, as a matlab string matrix
     * @param engine The matlab engine.
     * @param matrix The matrix object
     * @return A matlab string array.
     */
    matlab::data::Array export_sequence_matrix(matlab::engine::MATLABEngine& engine,
                                              const SymbolicMatrix& matrix);


    /**
     * Outputs as list of symbols associated with a matrix, and their corresponding real and imaginary indices.
     * @param engine The matlab engine.
     * @param smp The symbol matrix properties object.
     * @return 2xn or 3xn matlab array. First column are symbol ids, second real indices, third imaginary indices.
     */
    matlab::data::TypedArray<int32_t>
    export_basis_key(matlab::engine::MATLABEngine &engine, const MatrixProperties &smp);

    /**
     * Outputs a list of real and imaginary symbols associated with a matrix.
     * @param engine The matlab engine.
     * @param smp The symbol matrix properties object.
     * @return Pair of matlab arrays, first for real symbols, second for imaginary symbols.
     */
    std::pair<matlab::data::TypedArray<uint64_t>, matlab::data::TypedArray<uint64_t>>
        export_basis_lists(matlab::engine::MATLABEngine& engine, const MatrixProperties& smp);

    /**
     * Outputs masks for the real and imaginary symbols associated with a matrix.
     * If symbol i features in the matrix, element i-1 will have the value true. (In ML indexing, this corresponds to
     * element i); otherwise the value will be false. Symbol "0" is always omitted, as it never contributes to a basis.
     * @param engine The matlab engine.
     * @param symbol_table The total table of symbols.
     * @param smp The symbol matrix properties object.
     * @return Pair of matlab arrays, first for real symbols, second for imaginary symbols.
     */
    std::pair<matlab::data::TypedArray<bool>, matlab::data::TypedArray<bool>>
        export_basis_masks(matlab::engine::MATLABEngine& engine,
                           const SymbolTable& symbol_table,
                           const MatrixProperties& smp);


}
