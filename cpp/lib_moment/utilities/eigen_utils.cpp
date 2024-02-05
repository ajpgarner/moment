/**
 * eigen_utils.cpp
 *
 * @copyright Copyright (c) 2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "eigen_utils.h"

#include "float_utils.h"

namespace Moment {

    bool is_zero(const Eigen::MatrixXd& data, double zero_tolerance) noexcept {
        for (auto iter = data.data(); iter < data.data() + data.size(); ++iter) {
            if (!approximately_zero(*iter, zero_tolerance)) {
                return false;
            }
        }
        return true;
    }

    bool is_zero(const Eigen::MatrixXcd& data, double zero_tolerance) noexcept {
        for (auto iter = data.data(); iter < data.data() + data.size(); ++iter) {
            if (!approximately_zero(*iter, zero_tolerance)) {
                return false;
            }
        }
        return true;
    }

    bool is_zero(const Eigen::SparseMatrix<double>& data, double zero_tolerance) noexcept {
        return (data.nonZeros() == 0);
    }

    bool is_zero(const Eigen::SparseMatrix<std::complex<double>>& data, double zero_tolerance) noexcept {
        return (data.nonZeros() == 0);
    }

    bool is_hermitian(const Eigen::MatrixXcd& data, double zero_tolerance) noexcept {
        if (data.rows() != data.cols()) {
            return false;
        }

        for (size_t col = 0; col < data.cols(); ++col) {
            if (!approximately_real(data.coeff(col,col), zero_tolerance)) {
                return false;
            }
            for (size_t row = col+1; row < data.rows(); ++row) {
                if (!approximately_equal(std::conj(data.coeff(row, col)), data.coeff(col, row), zero_tolerance)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_hermitian(const Eigen::MatrixXd& data, double zero_tolerance) noexcept  {
        if (data.rows() != data.cols()) {
            return false;
        }

        for (size_t col = 0; col < data.cols(); ++col) {
            for (size_t row = col+1; row < data.rows(); ++row) {
                if (!approximately_equal(data.coeff(row, col), data.coeff(col, row), zero_tolerance)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_hermitian(const Eigen::SparseMatrix<double>& data, double zero_tolerance) noexcept  {
        using inner_iter_t = Eigen::SparseMatrix<double>::InnerIterator;
        if (data.rows() != data.cols()) {
            return false;
        }

        for (Eigen::Index outer = 0; outer < data.outerSize(); ++outer) {
            for  (inner_iter_t inner_iter{data, outer}; inner_iter; ++inner_iter) {
                if (inner_iter.row() > outer) {
                    if (!approximately_equal(data.coeff(inner_iter.col(), inner_iter.row()),
                                             inner_iter.value(), zero_tolerance)) {
                        return false;
                    }
                }
            }
        }

        return true;
    }

    bool is_hermitian(const Eigen::SparseMatrix<std::complex<double>>& data, double zero_tolerance) noexcept {
        using inner_iter_t = Eigen::SparseMatrix<std::complex<double>>::InnerIterator;

        if (data.rows() != data.cols()) {
            return false;
        }

        for (Eigen::Index outer = 0; outer < data.outerSize(); ++outer) {
            for  (inner_iter_t inner_iter{data, outer}; inner_iter; ++inner_iter) {
                if (inner_iter.row() == outer) {
                    if (!approximately_real(inner_iter.value(), zero_tolerance)) {
                        return false;
                    }
                } else if (inner_iter.row() > outer) {
                    if (!approximately_equal(std::conj(data.coeff(inner_iter.col(), inner_iter.row())),
                                             inner_iter.value(), zero_tolerance)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }

    bool is_antihermitian(const Eigen::MatrixXcd& data, double zero_tolerance) noexcept {
        if (data.rows() != data.cols()) {
            return false;
        }

        for (size_t col = 0; col < data.cols(); ++col) {
            if (!approximately_imaginary(data.coeff(col,col), zero_tolerance)) {
                return false;
            }
            for (size_t row = col+1; row < data.rows(); ++row) {
                if (!approximately_equal(-std::conj(data.coeff(row, col)), data.coeff(col, row), zero_tolerance)) {
                    return false;
                }
            }
        }
        return true;
    }

    bool is_antihermitian(const Eigen::SparseMatrix<std::complex<double>>& data, double zero_tolerance) noexcept {
        using inner_iter_t = Eigen::SparseMatrix<std::complex<double>>::InnerIterator;

        if (data.rows() != data.cols()) {
            return false;
        }

        for (Eigen::Index outer = 0; outer < data.outerSize(); ++outer) {
            for  (inner_iter_t inner_iter{data, outer}; inner_iter; ++inner_iter) {
                if (inner_iter.row() == outer) {
                    if (!approximately_imaginary(inner_iter.value(), zero_tolerance)) {
                        return false;
                    }
                } else if (inner_iter.row() > outer) {
                    if (!approximately_equal(-std::conj(data.coeff(inner_iter.col(), inner_iter.row())),
                                             inner_iter.value(), zero_tolerance)) {
                        return false;
                    }
                }
            }
        }
        return true;
    }
}