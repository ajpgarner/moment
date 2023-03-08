/**
 * read_eigen_sparse.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

namespace matlab::data {
    class Array;
}
namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    /** Reads a matlab matrix into a real eigen sparse matrix */
    Eigen::SparseMatrix<double> read_eigen_sparse(matlab::engine::MATLABEngine& engine,
                                                  const matlab::data::Array& array);

}