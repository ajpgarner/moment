/**
 * polynomial_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix.h"

#include "symbolic/polynomial.h"
#include "tensor/square_matrix.h"

#include <memory>

namespace Moment {

    class PolynomialMatrix : public Matrix {
    public:
        using ElementType = Polynomial;
        using MatrixData = SquareMatrix<Polynomial>;
        class PMSymbolMatrixView {
        private:
            const PolynomialMatrix& matrix;
        public:
            explicit PMSymbolMatrixView(const PolynomialMatrix& theMatrix) noexcept : matrix{theMatrix} { };

            [[nodiscard]] size_t Dimension() const noexcept { return matrix.Dimension(); }

            /**
            * Return a view (std::span<const Monomial>) to the requested row of the NPA matrix's symbolic
            * representation. Since std::span also provides an operator[], it is possible to index using
            * "mySMV[row][col]" notation.
            * @param row The index of the row to return.
            * @return A std::span<const Monomial> of the requested row.
            */
            std::span<const Polynomial> operator[](size_t row) const noexcept {
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
        std::unique_ptr<MatrixData> sym_exp_matrix;

    public:
        PolynomialMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                         std::unique_ptr<MatrixData> symbolMatrix);

        ~PolynomialMatrix() noexcept = default;

        /**
         * True if matrix is monomial in terms of symbols.
         */
        [[nodiscard]] bool is_monomial() const noexcept override {
            return false;
        }

        /**
         * Gets pointer to raw data
         */
        const Polynomial* raw_data() const noexcept {
            return this->sym_exp_matrix->raw();
        }

        /**
         * Force renumbering of matrix bases keys
         */
        void renumerate_bases(const SymbolTable& symbols,  double zero_tolerance) final;

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
         * Create sparse complex basis.
         */
        [[nodiscard]] SparseComplexBasisInfo::MakeStorageType create_sparse_complex_basis() const override;


        /**
         * Look up basis elements in matrix
         */
        void identify_symbols_and_basis_indices(double zero_tolerance);

    };
}