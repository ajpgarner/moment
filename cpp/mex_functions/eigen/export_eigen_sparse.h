/**
 * exported_eigen_sparse.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <Eigen/SparseCore>

#include <complex>
#include <vector>

namespace Moment::mex {
    /** Export real eigen sparse vector as MATLAB sparse matrix. */
    matlab::data::SparseArray<double>
    export_eigen_sparse(matlab::engine::MATLABEngine& engine,
                        matlab::data::ArrayFactory& factory,
                        const Eigen::SparseVector<double>& vector);

    /** Export real eigen sparse matrix as MATLAB sparse matrix. */
    matlab::data::SparseArray<double>
    export_eigen_sparse(matlab::engine::MATLABEngine& engine,
                        matlab::data::ArrayFactory& factory,
                        const Eigen::SparseMatrix<double>& matrix);

    /** Export complex eigen sparse matrix as MATLAB sparse matrix. */
    matlab::data::SparseArray<std::complex<double>>
    export_eigen_sparse(matlab::engine::MATLABEngine& engine,
                        matlab::data::ArrayFactory& factory,
                        const Eigen::SparseMatrix<std::complex<double>>& matrix);

    /** Export vector of real eigen sparse matrices as cell array of MATLAB sparse matrices. */
    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_sparse_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::SparseMatrix<double>>& matrices);

    /** Export vector of complex eigen sparse matrices as cell array of MATLAB sparse matrices. */
    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_sparse_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::SparseMatrix<std::complex<double>>>& matrices);
}