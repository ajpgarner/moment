/**
 * export_eigen_dense.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <Eigen/Dense>

namespace Moment::mex {
    matlab::data::TypedArray<double> export_eigen_dense(matlab::engine::MATLABEngine& engine,
                                                       const Eigen::MatrixXd& matrix);
}