/**
 * polynomial_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "matrix.h"

#include "symbolic/polynomial.h"
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
        std::unique_ptr<SquareMatrix<Polynomial>> sym_exp_matrix;

        /** Complex? */
        bool real_prefactors = true;

    public:
        PolynomialMatrix(const Context& context, SymbolTable& symbols,
                         std::unique_ptr<SquareMatrix<Polynomial>> symbolMatrix);

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
         * True if matrix has no complex prefactors in any of its constituent polynomials.
         */
        bool real_coefficients() const noexcept override {
            return this->real_prefactors;
        }

        /**
         * Force renumbering of matrix bases keys
         */
        void renumerate_bases(const SymbolTable& symbols) override;

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
    };
}