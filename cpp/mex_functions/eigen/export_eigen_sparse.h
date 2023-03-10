/**
 * exported_eigen_sparse.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <Eigen/Sparse>

#include <vector>

namespace Moment::mex {
    /** Export eigen sparse matrix as MATLAB sparse matrix. */
    matlab::data::SparseArray<double> export_eigen_sparse(matlab::engine::MATLABEngine& engine,
                                                          matlab::data::ArrayFactory& factory,
                                                          const Eigen::SparseMatrix<double>& matrix);

    /** Export vector of eigen sparse matrices as cell array of MATLAB sparse matrices. */
    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_sparse_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::SparseMatrix<double>>& matrices);
}