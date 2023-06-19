/**
 * monomial_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix.h"

#include "symbolic/monomial.h"
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
            * Return a view (std::span<const Monomial>) to the requested row of the NPA matrix's symbolic
            * representation. Since std::span also provides an operator[], it is possible to index using
            * "mySMV[row][col]" notation.
            * @param row The index of the row to return.
            * @return A std::span<const Monomial> of the requested row.
            */
            std::span<const Monomial> operator[](size_t row) const noexcept {
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
        std::unique_ptr<SquareMatrix<Monomial>> sym_exp_matrix;

    public:
        MonomialMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                       std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix,
                       bool is_hermitian);

        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix);

        ~MonomialMatrix() noexcept;

        /**
         * Force renumbering of matrix bases keys
         */
        void renumerate_bases(const SymbolTable& symbols, double zero_tolerance) final;


    protected:

        /**
         * Create dense basis.
         */
        [[nodiscard]] DenseBasisInfo::MakeStorageType create_dense_basis() const override;

        /**
         * Create sparse basis.
         */
        [[nodiscard]] SparseBasisInfo::MakeStorageType create_sparse_basis() const override;

        /**
         * Create dense complex basis.
         */
        [[nodiscard]] DenseComplexBasisInfo::MakeStorageType create_dense_complex_basis() const override;

        /**
         * Create sparse basis.
         */
        [[nodiscard]] SparseComplexBasisInfo::MakeStorageType create_sparse_complex_basis() const override;

        /**
         * Look up basis elements in matrix
         */
        void identify_symbols_and_basis_indices();

    };
}