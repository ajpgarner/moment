/**
 * monomial_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix.h"

#include "symbolic/symbol_expression.h"
#include "utilities/square_matrix.h"

#include <memory>

namespace Moment {

    class SymbolTable;
    class OperatorMatrix;
    class MatrixProperties;

    /**
     * Symbolic matrix, where each entry represents a monomial expression.
     */
    class MonomialMatrix : public Matrix {
    public:
        class MMSymbolMatrixView {
        private:
            const MonomialMatrix& matrix;
        public:
            explicit MMSymbolMatrixView(const MonomialMatrix& theMatrix) noexcept : matrix{theMatrix} { };

            [[nodiscard]] size_t Dimension() const noexcept { return matrix.Dimension(); }

            /**
            * Return a view (std::span<const SymbolExpression>) to the requested row of the NPA matrix's symbolic
            * representation. Since std::span also provides an operator[], it is possible to index using
            * "mySMV[row][col]" notation.
            * @param row The index of the row to return.
            * @return A std::span<const SymbolExpression> of the requested row.
            */
            std::span<const SymbolExpression> operator[](size_t row) const noexcept {
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
        std::unique_ptr<SquareMatrix<SymbolExpression>> sym_exp_matrix;

    public:
        MonomialMatrix(SymbolTable& symbols, const Context& context,
                       std::unique_ptr<SquareMatrix<SymbolExpression>> symbolMatrix,
                       bool is_hermitian);

        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix);

        MonomialMatrix(MonomialMatrix&& rhs) noexcept
            : Matrix{std::move(rhs)},
              sym_exp_matrix{std::move(rhs.sym_exp_matrix)},  SymbolMatrix{*this} { }

        ~MonomialMatrix() noexcept;

        /**
         * Force renumbering of matrix bases keys
         */
        void renumerate_bases(const SymbolTable& symbols) override;

    protected:

        /**
         * Create dense basis.
         */
        [[nodiscard]] std::pair<MatrixBasis::dense_real_storage_t, MatrixBasis::dense_complex_storage_t>
        create_dense_basis() const override;

        /**
         * Create sparse basis.
         */
        [[nodiscard]] std::pair<MatrixBasis::sparse_real_storage_t, MatrixBasis::sparse_complex_storage_t>
        create_sparse_basis() const override;



    };
}