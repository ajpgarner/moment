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

#include <complex>
#include <vector>

namespace Moment::mex {

    /** Export eigen real dense matrix as MATLAB real dense matrix */
    matlab::data::TypedArray<double>
    export_eigen_dense(matlab::engine::MATLABEngine& engine,
                       matlab::data::ArrayFactory& factory,
                       const Eigen::MatrixXd& matrix);

    /** Export eigen complex dense matrix as MATLAB complex dense matrix */
    matlab::data::TypedArray<std::complex<double>>
    export_eigen_dense(matlab::engine::MATLABEngine& engine,
                       matlab::data::ArrayFactory& factory,
                       const Eigen::MatrixXcd& matrix);

    /** Export vector of real eigen sparse matrices as cell array of MATLAB sparse matrices. */
    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_dense_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::MatrixXd>& matrices);

    /** Export vector of complex eigen sparse matrices as cell array of MATLAB sparse matrices. */
    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_dense_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::MatrixXcd>& matrices);
}