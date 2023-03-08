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

namespace Moment::mex {
    matlab::data::SparseArray<double> export_eigen_sparse(matlab::engine::MATLABEngine& engine,
                                                          const Eigen::SparseMatrix<double>& matrix);
}