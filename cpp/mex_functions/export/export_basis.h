/**
 * export_basis.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"
#include "exporter.h"

#include <complex>
#include <utility>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class SymbolicMatrix;
}


namespace Moment::mex {

    class BasisExporter : public ExporterWithFactory {
    public:
        const bool Sparse;
        const bool Monolithic;
    public:
        /**
         * Create object that exports bases from supplied symbol matrices.
         * @param engine The MATLAB engine
         * @param sparse True to export as sparse matrices
         * @param monolithic True to export as a single giant matrix, false to export as a cell array.
         */
        explicit BasisExporter(matlab::engine::MATLABEngine& engine, bool sparse = false, bool monolithic = false)
            : ExporterWithFactory{engine}, Sparse{sparse}, Monolithic{monolithic} { }

        /**
        * Exports basis of matrix in requested format. Will infer if complex parts are necessary.
        * @param matrix The matrix whose basis to output.
        */
        [[nodiscard]] std::pair<matlab::data::Array, matlab::data::Array>
        operator()(const SymbolicMatrix &matrix) const;

    };
}
