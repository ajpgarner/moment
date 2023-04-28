/**
 * read_eigen_sparse.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

#include <vector>

namespace matlab::data {
    class Array;
}
namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    /** Reads a matlab matrix into a real eigen sparse matrix. */
    Eigen::SparseMatrix<double> read_eigen_sparse(matlab::engine::MATLABEngine& engine,
                                                  const matlab::data::Array& array);


    /** Reads a matlab matrix into a real eigen sparse matrix. */
    Eigen::SparseVector<double> read_eigen_sparse_vector(matlab::engine::MATLABEngine& engine,
                                                         const matlab::data::Array& array);

    /** Reads a matlab cell array into a vector of real eigen sparse matrices. */
    std::vector<Eigen::SparseMatrix<double>>
    read_eigen_sparse_array(matlab::engine::MATLABEngine& engine, const matlab::data::Array& array);

}