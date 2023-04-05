/**
 * export_basis.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_basis.h"

#include "matrix/matrix.h"

#include "eigen/export_eigen_dense.h"
#include "eigen/export_eigen_sparse.h"

#include <complex>

namespace Moment::mex {

    std::pair<matlab::data::CellArray, matlab::data::CellArray>
    export_dense_cell_basis(matlab::engine::MATLABEngine &engine, const Matrix& mm) {

        const auto& [re_basis, im_basis] = mm.Basis.Dense();

        matlab::data::ArrayFactory factory{};//

        return std::make_pair(export_eigen_dense_array(engine, factory, re_basis),
                             export_eigen_dense_array(engine, factory, im_basis));

    }

    std::pair<matlab::data::CellArray, matlab::data::CellArray>
    export_sparse_cell_basis(matlab::engine::MATLABEngine &engine,
                                       const Matrix& mm) {
        const auto& [re_basis, im_basis] = mm.Basis.Sparse();

        matlab::data::ArrayFactory factory{};//

        return std::make_pair(export_eigen_sparse_array(engine, factory, re_basis),
                              export_eigen_sparse_array(engine, factory, im_basis));
    }

    std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>
    export_dense_monolith_basis(matlab::engine::MATLABEngine &engine,
                                          const Matrix& mm) {
        const auto& [re_basis, im_basis] = mm.Basis.DenseMonolithic();

        matlab::data::ArrayFactory factory{};
        return std::make_pair(export_eigen_dense(engine, factory, re_basis),
                              export_eigen_dense(engine, factory, im_basis));

    }

    std::pair<matlab::data::SparseArray<double>, matlab::data::SparseArray<std::complex<double>>>
    export_sparse_monolith_basis(matlab::engine::MATLABEngine &engine, const Matrix& mm) {
        const auto& [re_basis, im_basis] = mm.Basis.SparseMonolithic();

        matlab::data::ArrayFactory factory{};
        return std::make_pair(export_eigen_sparse(engine, factory, re_basis),
                              export_eigen_sparse(engine, factory, im_basis));
    }



}