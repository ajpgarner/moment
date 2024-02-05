/**
 * eigen_utils.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <complex>
#include <memory>

namespace Moment {
    /**
     * True, if data is Symmetric (equal to transpose). Equivalently, Hermitian.
     */
    [[nodiscard]] bool is_hermitian(const Eigen::MatrixXd& data,
                                    double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is Hermitian (equal to conjugate transpose).
     */
    [[nodiscard]] bool is_hermitian(const Eigen::MatrixXcd& data,
                                    double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is Symmetric (equal to transpose). Equivalently, Hermitian.
     */
    [[nodiscard]] bool is_hermitian(const Eigen::SparseMatrix<double>& data,
                                    double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is Hermitian (equal to conjugate transpose).
     */
    [[nodiscard]] bool is_hermitian(const Eigen::SparseMatrix<std::complex<double>>& data,
                                    double zero_tolerance = 1.0) noexcept;

}