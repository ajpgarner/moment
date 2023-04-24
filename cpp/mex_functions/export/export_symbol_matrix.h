/**
 * export_symbol_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "exporter.h"

#include "MatlabDataArray.hpp"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class MonomialMatrix;
    class PolynomialMatrix;
}

namespace Moment::mex {


    class SymbolMatrixExporter : public Exporter {
    public:
        explicit SymbolMatrixExporter(matlab::engine::MATLABEngine& engine) : Exporter{engine} { }

        /**
        * Outputs a matrix of symbol expressions, as a matlab string matrix
        * @param engine The matlab engine.
        * @param matrix The matrix of symbols to output.
        * @return A matlab string array.
        */
        [[nodiscard]] matlab::data::Array operator()(const MonomialMatrix &matrix) const;

        /**
        * Outputs a matrix of symbol expressions, as a matlab string matrix
        * @param engine The matlab engine.
        * @param matrix The matrix of symbols to output.
        * @return A matlab string array.
        */
        [[nodiscard]] matlab::data::Array operator()(const PolynomialMatrix &matrix) const;
    };


}