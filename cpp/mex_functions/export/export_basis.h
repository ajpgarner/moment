/**
 * export_basis.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "MatlabDataArray.hpp"

#include <complex>
#include <utility>

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment {
    class SymbolicMatrix;
}

namespace Moment::mex {
    std::pair<matlab::data::CellArray, matlab::data::CellArray>
    export_dense_cell_basis(matlab::engine::MATLABEngine &engine, const SymbolicMatrix& mm);

    std::pair<matlab::data::CellArray, matlab::data::CellArray>
    export_sparse_cell_basis(matlab::engine::MATLABEngine &engine, const SymbolicMatrix& mm);

    std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>
    export_dense_monolith_basis(matlab::engine::MATLABEngine &engine,const SymbolicMatrix& mm);

    std::pair<matlab::data::SparseArray<double>, matlab::data::SparseArray<std::complex<double>>>
    export_sparse_monolith_basis(matlab::engine::MATLABEngine &engine,const SymbolicMatrix& mm);
}
