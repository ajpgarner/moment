/**
 * eigen_sparse_kron_pow.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>


namespace Moment {
    /**
     * Take the kronecker product of base with itself, power times.
     * @param base The base matrix.
     * @param power The integral power.
     * @return Kronecker product power.
     */
    Eigen::SparseMatrix<double> kronecker_power(const Eigen::SparseMatrix<double>& base, int power);
}