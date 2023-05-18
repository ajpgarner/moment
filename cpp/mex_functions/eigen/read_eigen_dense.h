/**
 * read_eigen_dense.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Dense>

namespace matlab::data {
    class Array;
}
namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {

    /** Reads a matlab matrix into a real eigen dense matrix */
    Eigen::MatrixXd read_eigen_dense(matlab::engine::MATLABEngine& engine,
                                     const matlab::data::Array& array);

    /** Reads a matlab matrix into a complex eigen dense matrix */
    Eigen::MatrixXcd read_eigen_dense_complex(matlab::engine::MATLABEngine& engine,
                                             const matlab::data::Array& array);

}