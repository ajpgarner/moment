/**
 * polynomial_matrix_basis.cpp
 *
 * Additional functions relating to basis generation for PolynomialMatrix.
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "polynomial_matrix.h"

#include "symbolic/symbol_table.h"

namespace Moment {
    namespace {


        template<typename MatrixInfo>
        inline typename MatrixInfo::RealMatrixType::Scalar get_re_factor(std::complex<double> val) {
            return val;
        }

        template<>
        inline double get_re_factor<DenseBasisInfo>(std::complex<double> val) {
            return val.real();
        }

        template<>
        inline double get_re_factor<SparseBasisInfo>(std::complex<double> val) {
            return val.real();
        }


        /**
         * Create dense basis from polynomial matrix.
         * @tparam symmetric True if matrix is symmetric/hermitian.
         * @tparam complex True if matrix has imaginary elements.
         * @param symbols Symbol table.
         * @param matrix The matrix.
         * @param real Output: real matrix basis.
         * @param im Output: imaginary matrix basis.
         */
        template<typename BasisInfo, bool symmetric, bool complex>
        void do_create_dense_basis_impl(const SymbolTable& symbols,
                                        const SquareMatrix<Polynomial>& matrix,
                                        typename BasisInfo::RealStorageType& real,
                                        typename BasisInfo::ImStorageType& im) {
            const auto dimension = matrix.dimension;
            for (int row_index = 0; row_index < dimension; ++row_index) {
                for (int col_index = symmetric ? row_index : 0; col_index < dimension; ++col_index) {
                    // TODO move out of loop
                    const std::array<size_t, 2> index{static_cast<size_t>(row_index), static_cast<size_t>(col_index)};
                    const auto &poly = matrix(index);
                    for (const auto& elem : poly) {

                        assert(elem.id < symbols.size());
                        auto [re_id, im_id] = symbols[elem.id].basis_key();

                        if (re_id >= 0) {
                            assert(re_id < real.size());
                            real[re_id](row_index, col_index) += get_re_factor<BasisInfo>(elem.factor);
                            if constexpr(symmetric) {
                                if (row_index != col_index) {
                                    real[re_id](col_index, row_index) += get_re_factor<BasisInfo>(std::conj(elem.factor));
                                }
                            }
                        }

                        if constexpr(complex) {
                            if (im_id >= 0) {
                                assert(im_id < im.size());

                                im[im_id](row_index, col_index) +=
                                        std::complex<double>(0.0, (elem.conjugated ? -1.0 : 1.0)) * elem.factor;
                                if constexpr(symmetric) {
                                    if (row_index != col_index) {
                                        im[im_id](col_index, row_index) +=
                                                std::complex<double>(0.0, (elem.conjugated ? 1.0 : -1.0)) * std::conj(elem.factor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        template<typename BasisInfo>
        typename BasisInfo::MakeStorageType
        do_create_dense_matrix(const PolynomialMatrix& matrix) {
            typename BasisInfo::MakeStorageType output;
            auto& real = output.first;
            auto& im = output.second;

            auto dim = static_cast<typename BasisInfo::IndexType>(matrix.Dimension());

            real.assign(matrix.symbols.Basis.RealSymbolCount(), BasisInfo::RealMatrixType::Zero(dim, dim));
            im.assign(matrix.symbols.Basis.ImaginarySymbolCount(), BasisInfo::ImMatrixType::Zero(dim, dim));

            const bool symmetric = matrix.Hermitian();
            const bool complex = matrix.HasComplexBasis();

            if (symmetric) {
                if (complex) {
                    do_create_dense_basis_impl<BasisInfo, true, true>(matrix.symbols, matrix.SymbolMatrix(), real, im);
                } else {
                    do_create_dense_basis_impl<BasisInfo, true, false>(matrix.symbols, matrix.SymbolMatrix(), real, im);
                }
            } else {
                if (complex) {
                    do_create_dense_basis_impl<BasisInfo, false, true>(matrix.symbols, matrix.SymbolMatrix(), real, im);
                } else {
                    do_create_dense_basis_impl<BasisInfo, false, false>(matrix.symbols, matrix.SymbolMatrix(), real, im);
                }
            }
            return output;
        }

        template<typename BasisInfo, bool symmetric, bool complex>
        void do_create_sparse_frame(const SymbolTable& symbols,
                                    const SquareMatrix<Polynomial>& matrix,
                                    std::vector<std::vector<typename BasisInfo::RealTripletType>>& real_frame,
                                    std::vector<std::vector<typename BasisInfo::ImTripletType>>& im_frame) {

            const auto dimension = static_cast<int>(matrix.dimension);
            for (int row_index = 0; row_index < dimension; ++row_index) {
                for (int col_index = symmetric ? row_index : 0; col_index < dimension; ++col_index) {
                    //TODO move out of loop
                    const std::array<size_t, 2> index{static_cast<size_t>(row_index), static_cast<size_t>(col_index)};
                    const auto &poly = matrix(index);
                    for (const auto &elem : poly) {
                        assert(elem.id < symbols.size());
                        auto [re_id, im_id] = symbols[elem.id].basis_key();

                        if (re_id >= 0) {
                            assert(re_id < real_frame.size());
                            real_frame[re_id].emplace_back(row_index, col_index, get_re_factor<BasisInfo>(elem.factor));
                            if constexpr (symmetric) {
                                if (row_index != col_index) {
                                    real_frame[re_id].emplace_back(col_index, row_index,
                                                                   get_re_factor<BasisInfo>(std::conj(elem.factor)));
                                }
                            }
                        }

                        if constexpr (complex) {
                            if (im_id >= 0) {
                                assert(im_id < im_frame.size());
                                im_frame[im_id].emplace_back(row_index, col_index,
                                                             std::complex<double>(0, (elem.conjugated ? -1.0 : 1.0))
                                                             * elem.factor);
                                if constexpr (symmetric) {
                                    if (row_index != col_index) {
                                        im_frame[im_id].emplace_back(col_index, row_index,
                                                                     std::complex<double>(0, (elem.conjugated ? 1.0 : -1.0))
                                                                     * std::conj(elem.factor));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }

        template<typename BasisInfo>
        typename BasisInfo::MakeStorageType do_create_sparse_basis(const PolynomialMatrix& matrix)  {

            // Get matrix properties
            const auto dim = static_cast<typename BasisInfo::IndexType>(matrix.Dimension());
            const bool symmetric = matrix.Hermitian();
            const bool complex = matrix.HasComplexBasis();

            // Prepare triplets
            std::vector<std::vector<typename BasisInfo::RealTripletType>> real_frame(matrix.symbols.Basis.RealSymbolCount());
            std::vector<std::vector<typename BasisInfo::ImTripletType>> im_frame(matrix.symbols.Basis.ImaginarySymbolCount());

            if (symmetric) {
                if (complex) {
                    do_create_sparse_frame<BasisInfo, true, true>(matrix.symbols, matrix.SymbolMatrix(),
                                                                  real_frame, im_frame);
                } else {
                    do_create_sparse_frame<BasisInfo, true, false>(matrix.symbols, matrix.SymbolMatrix(),
                                                                   real_frame, im_frame);
                }
            } else {
                if (complex) {
                    do_create_sparse_frame<BasisInfo, false, true>(matrix.symbols, matrix.SymbolMatrix(),
                                                                   real_frame, im_frame);
                } else {
                    do_create_sparse_frame<BasisInfo, false, false>(matrix.symbols, matrix.SymbolMatrix(),
                                                                    real_frame, im_frame);
                }
            }


            // Now, build sparse matrices
            typename BasisInfo::MakeStorageType output;
            auto& real = output.first;
            real.assign(real_frame.size(), typename BasisInfo::RealMatrixType(dim, dim));
            for (size_t re_index = 0; re_index < real_frame.size(); ++re_index) {
                real[re_index].setFromTriplets(real_frame[re_index].cbegin(), real_frame[re_index].cend());
            }

            if (complex) {
                auto &im = output.second;
                im.assign(im_frame.size(), typename BasisInfo::ImMatrixType(dim, dim));
                for (size_t im_index = 0; im_index < im_frame.size(); ++im_index) {
                    im[im_index].setFromTriplets(im_frame[im_index].cbegin(), im_frame[im_index].cend());
                }
            } else {
                // Null case: symbols are complex, but matrix is not.
                if (matrix.symbols.Basis.ImaginarySymbolCount() > 0) {
                    output.second.assign(matrix.symbols.Basis.ImaginarySymbolCount(),
                                         typename BasisInfo::ImMatrixType(dim, dim));
                }
            }
            return output;
        }
    }

    DenseBasisInfo::MakeStorageType PolynomialMatrix::create_dense_basis() const {
        return do_create_dense_matrix<DenseBasisInfo>(*this);
    }

    DenseComplexBasisInfo::MakeStorageType PolynomialMatrix::create_dense_complex_basis() const {
        return do_create_dense_matrix<DenseComplexBasisInfo>(*this);
    }

    SparseBasisInfo::MakeStorageType PolynomialMatrix::create_sparse_basis() const {
        return do_create_sparse_basis<SparseBasisInfo>(*this);
    }

    SparseComplexBasisInfo::MakeStorageType PolynomialMatrix::create_sparse_complex_basis() const {
        return do_create_sparse_basis<SparseComplexBasisInfo>(*this);
    }
}