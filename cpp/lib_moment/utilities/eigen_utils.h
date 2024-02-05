/**
 * eigen_utils.h
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/Core>
#include <Eigen/SparseCore>

#include <complex>
#include <memory>

namespace Moment {


    /**
     * True, if data is all zero.
     */
    [[nodiscard]] bool is_zero(const Eigen::MatrixXd& data,
                               double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is all zero.
     */
    [[nodiscard]] bool is_zero(const Eigen::MatrixXcd& data,
                               double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is all zero.
     */
    [[nodiscard]] bool is_zero(const Eigen::SparseMatrix<double>& data,
                               double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is all zero.
     */
    [[nodiscard]] bool is_zero(const Eigen::SparseMatrix<std::complex<double>>& data,
                               double zero_tolerance = 1.0) noexcept;

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


     /**
     * True, if data is anti-hermitian (equal to negated conjugate transpose).
     * For real-valued matrices, equivalent to testing if matrix is zero.
     */
    [[nodiscard]] inline bool is_antihermitian(const Eigen::MatrixXd& data,
                                    double zero_tolerance = 1.0) noexcept {
        return is_zero(data, zero_tolerance);
    }

    /**
     * True, if data is anti-hermitian (equal to negated conjugate transpose).
     */
    [[nodiscard]] bool is_antihermitian(const Eigen::MatrixXcd& data,
                                    double zero_tolerance = 1.0) noexcept;

    /**
     * True, if data is anti-hermitian (equal to negated conjugate transpose).
     * For real-valued matrices, equivalent to testing if matrix is zero.
     */
    [[nodiscard]] inline bool is_antihermitian(const Eigen::SparseMatrix<double>& data,
                                        double zero_tolerance = 1.0) noexcept {
        return is_zero(data, zero_tolerance);
    }

    /**
     * True, if data is anti-hermitian (equal to negated conjugate transpose).
     */
    [[nodiscard]] bool is_antihermitian(const Eigen::SparseMatrix<std::complex<double>>& data,
                                        double zero_tolerance = 1.0) noexcept;




}