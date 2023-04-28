/**
 * polynomial_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_matrix.h"

#include "symbolic/symbol_table.h"

namespace Moment {

    namespace {

        bool test_hermicity(const SymbolTable &table, const SquareMatrix<SymbolCombo> &matrix) {

            for (size_t row = 0; row < matrix.dimension; ++row) {
                if (!matrix[row][row].is_hermitian(table)) {
                    return false;
                }

                for (size_t col = row+1; col < matrix.dimension; ++col) {
                    const auto& upper = matrix[row][col];
                    const auto& lower = matrix[col][row];
                    if (!upper.is_conjugate(table, lower)) {
                        return false;
                    }
                }
            }

            return true;
        }


        using re_trip_t = Eigen::Triplet<sparse_real_elem_t::value_type>;
        using im_trip_t = Eigen::Triplet<sparse_complex_elem_t::value_type>;


        /**
         * Create dense basis from polynomial matrix.
         * @tparam symmetric True if matrix is symmetric/hermitian.
         * @tparam complex True if matrix has imaginary elements.
         * @param symbols Symbol table.
         * @param matrix The matrix.
         * @param real Output: real matrix basis.
         * @param im Output: imaginary matrix basis.
         */
        template<bool symmetric, bool complex>
        void do_create_dense_basis(const SymbolTable& symbols,
                                   const SquareMatrix<SymbolCombo>& matrix,
                                   Matrix::MatrixBasis::dense_real_storage_t& real,
                                   Matrix::MatrixBasis::dense_complex_storage_t& im) {
            const auto dimension = matrix.dimension;
            for (int row_index = 0; row_index < dimension; ++row_index) {
                for (int col_index = symmetric ? row_index : 0; col_index < dimension; ++col_index) {
                    const auto &poly = matrix[row_index][col_index];
                    for (const auto& elem : poly) {

                        assert(elem.id < symbols.size());
                        auto [re_id, im_id] = symbols[elem.id].basis_key();

                        if (re_id >= 0) {
                            assert(re_id < real.size());
                            real[re_id](row_index, col_index) += elem.factor;
                            if constexpr(symmetric) {
                                if (row_index != col_index) {
                                    real[re_id](col_index, row_index) += elem.factor;
                                }
                            }
                        }

                        if constexpr(complex) {
                            if (im_id >= 0) {
                                assert(im_id < im.size());

                                im[im_id](row_index, col_index) +=
                                        std::complex<double>(0.0, (elem.conjugated ? -1.0 : 1.0) * elem.factor);
                                if constexpr(symmetric) {
                                    if (row_index != col_index) {
                                        im[im_id](col_index, row_index) +=
                                                std::complex<double>(0.0, (elem.conjugated ? 1.0 : -1.0) * elem.factor);
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }


        template<bool symmetric, bool complex>
        void do_create_sparse_frame(const SymbolTable& symbols,
                                    const SquareMatrix<SymbolCombo>& matrix,
                                    std::vector<std::vector<re_trip_t>>& real_frame,
                                    std::vector<std::vector<im_trip_t>>& im_frame) {

            const auto dimension = static_cast<int>(matrix.dimension);
            for (int row_index = 0; row_index < dimension; ++row_index) {
                for (int col_index = symmetric ? row_index : 0; col_index < dimension; ++col_index) {
                    const auto &poly = matrix[row_index][col_index];
                    for (const auto &elem : poly) {
                        assert(elem.id < symbols.size());
                        auto [re_id, im_id] = symbols[elem.id].basis_key();

                        if (re_id >= 0) {
                            assert(re_id < real_frame.size());
                            real_frame[re_id].emplace_back(row_index, col_index, elem.factor);
                            if constexpr (symmetric) {
                                if (row_index != col_index) {
                                    real_frame[re_id].emplace_back(col_index, row_index, elem.factor);
                                }
                            }
                        }

                        if constexpr (complex) {
                            if (im_id >= 0) {
                                assert(im_id < im_frame.size());
                                im_frame[im_id].emplace_back(row_index, col_index,
                                     std::complex<double>(0, (elem.conjugated ? -1.0 : 1.0) * elem.factor));
                                if constexpr (symmetric) {
                                    if (row_index != col_index) {
                                        im_frame[im_id].emplace_back(col_index, row_index,
                                             std::complex<double>(0, (elem.conjugated ? 1.0 : -1.0) * elem.factor));
                                    }
                                }
                            }
                        }
                    }
                }
            }
        }
    }

    PolynomialMatrix::PolynomialMatrix(const Context& context, SymbolTable& symbols,
                     std::unique_ptr<SquareMatrix<SymbolCombo>> symbolMatrix)
             : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0}, SymbolMatrix{*this},
               sym_exp_matrix{std::move(symbolMatrix)} {
        if (!sym_exp_matrix) {
            throw std::runtime_error{"Symbol matrix pointer passed to PolynomialMatrix constructor was nullptr."};
        }

        // Find included symbols
        std::set<symbol_name_t> included_symbols;
        const size_t max_symbol_id = symbols.size();
        for (const auto& poly : *sym_exp_matrix) {
            for (const auto& term : poly) {
                included_symbols.emplace(term.id);
            }
        }

        // Test for Hermiticity
        const bool is_hermitian = test_hermicity(symbols, *sym_exp_matrix);

        // Create symbol matrix properties
        this->mat_prop = std::make_unique<MatrixProperties>(*this, this->symbol_table, std::move(included_symbols),
                                                            "Polynomial Symbolic Matrix", is_hermitian);

    }


    std::pair<Matrix::MatrixBasis::dense_real_storage_t, Matrix::MatrixBasis::dense_complex_storage_t>
    PolynomialMatrix::create_dense_basis() const {
        std::pair<Matrix::MatrixBasis::dense_real_storage_t, Matrix::MatrixBasis::dense_complex_storage_t> output;
        auto& real = output.first;
        auto& im = output.second;

        auto dim = static_cast<dense_real_elem_t::Index>(this->dimension);

        real.assign(this->symbol_table.Basis.RealSymbolCount(),
                    dense_real_elem_t::Zero(dim, dim));
        im.assign(this->symbol_table.Basis.ImaginarySymbolCount(),
                  dense_real_elem_t::Zero(dim, dim));

        const bool symmetric = this->SMP().IsHermitian();
        const bool complex = this->SMP().IsComplex();

        if (symmetric) {
            if (complex) {
                do_create_dense_basis<true, true>(this->Symbols, *this->sym_exp_matrix, real, im);
            } else {
                do_create_dense_basis<true, false>(this->Symbols, *this->sym_exp_matrix, real, im);
            }
        } else {
            if (complex) {
                do_create_dense_basis<false, true>(this->Symbols, *this->sym_exp_matrix, real, im);
            } else {
                do_create_dense_basis<false, false>(this->Symbols, *this->sym_exp_matrix, real, im);
            }
        }
        return output;
    }

    std::pair<Matrix::MatrixBasis::sparse_real_storage_t, Matrix::MatrixBasis::sparse_complex_storage_t>
    PolynomialMatrix::create_sparse_basis() const {
        // Get matrix properties
        const auto dim = static_cast<sparse_real_elem_t::Index>(this->dimension);
        const bool symmetric = this->SMP().IsHermitian();
        const bool complex = this->SMP().IsComplex();

        // Prepare triplets
        std::vector<std::vector<re_trip_t>> real_frame(this->Symbols.Basis.RealSymbolCount());
        std::vector<std::vector<im_trip_t>> im_frame(this->Symbols.Basis.ImaginarySymbolCount());

        if (symmetric) {
            if (complex) {
                do_create_sparse_frame<true, true>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            } else {
                do_create_sparse_frame<true, false>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            }
        } else {
            if (complex) {
                do_create_sparse_frame<false, true>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            } else {
                do_create_sparse_frame<false, false>(this->Symbols, *this->sym_exp_matrix, real_frame, im_frame);
            }
        }


        // Now, build sparse matrices
        std::pair<Matrix::MatrixBasis::sparse_real_storage_t, Matrix::MatrixBasis::sparse_complex_storage_t> output;
        auto& real = output.first;
        real.assign(real_frame.size(), sparse_real_elem_t(dim, dim));
        for (size_t re_index = 0; re_index < real_frame.size(); ++re_index) {
            real[re_index].setFromTriplets(real_frame[re_index].cbegin(), real_frame[re_index].cend());
        }

        if (complex) {
            auto &im = output.second;
            im.assign(im_frame.size(), sparse_complex_elem_t(dim, dim));
            for (size_t im_index = 0; im_index < im_frame.size(); ++im_index) {
                im[im_index].setFromTriplets(im_frame[im_index].cbegin(), im_frame[im_index].cend());
            }
        }

        return output;
    }

    /**
     * Force renumbering of matrix bases keys
     */
    void PolynomialMatrix::renumerate_bases(const SymbolTable& symbols) {
        // TODO: Support renumbering for polynomial matrix
        throw std::runtime_error{"PolynomialMatrix::renumerate_bases not implemented."};
    }
}