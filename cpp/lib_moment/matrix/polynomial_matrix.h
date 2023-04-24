/**
 * polynomial_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix.h"

#include "symbolic/symbol_combo.h"
#include "utilities/square_matrix.h"

#include <memory>

namespace Moment {

    class PolynomialMatrix : public Matrix {
    public:
        class PMSymbolMatrixView {
        private:
            const PolynomialMatrix& matrix;
        public:
            explicit PMSymbolMatrixView(const PolynomialMatrix& theMatrix) noexcept : matrix{theMatrix} { };

            [[nodiscard]] size_t Dimension() const noexcept { return matrix.Dimension(); }

            /**
            * Return a view (std::span<const SymbolExpression>) to the requested row of the NPA matrix's symbolic
            * representation. Since std::span also provides an operator[], it is possible to index using
            * "mySMV[row][col]" notation.
            * @param row The index of the row to return.
            * @return A std::span<const SymbolExpression> of the requested row.
            */
            std::span<const SymbolCombo> operator[](size_t row) const noexcept {
                return (*(matrix.sym_exp_matrix))[row];
            };

            /**
             * Provides access to square matrix of symbols.
             */
            const auto& operator()() const noexcept {
                return (*(matrix.sym_exp_matrix));
            }

        } SymbolMatrix;


    protected:
        /** Matrix, as symbolic expression */
        std::unique_ptr<SquareMatrix<SymbolCombo>> sym_exp_matrix;

    public:
        PolynomialMatrix(const Context& context, SymbolTable& symbols,
                         std::unique_ptr<SquareMatrix<SymbolCombo>> symbolMatrix);

        PolynomialMatrix(PolynomialMatrix&& rhs) noexcept : Matrix{std::move(rhs)},
            sym_exp_matrix{std::move(rhs.sym_exp_matrix)},  SymbolMatrix{*this} { }

        ~PolynomialMatrix() noexcept = default;

        /**
         * True if matrix is monomial in terms of symbols.
         */
        [[nodiscard]] bool is_monomial() const noexcept override {
            return false;
        }

        /**
         * Force renumbering of matrix bases keys
         */
        void renumerate_bases(const SymbolTable& symbols) override;

    protected:
        std::pair<MatrixBasis::dense_real_storage_t, MatrixBasis::dense_complex_storage_t>
        create_dense_basis() const override;

        std::pair<MatrixBasis::sparse_real_storage_t, MatrixBasis::sparse_complex_storage_t>
        create_sparse_basis() const override;
    };
}