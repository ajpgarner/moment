/**
 * monomial_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "symbolic_matrix.h"

#include "symbolic/monomial.h"
#include "tensor/square_matrix.h"

#include <complex>
#include <memory>
#include <stdexcept>

namespace Moment {

    class OperatorMatrix;
    class SymbolTable;

    /**
     * Symbolic matrix, where each entry represents a monomial expression.
     */
    class MonomialMatrix : public SymbolicMatrix {
    public:
        using ElementType = Monomial;

        class MMSymbolMatrixView {
        private:
            const MonomialMatrix& matrix;
        public:
            explicit MMSymbolMatrixView(const MonomialMatrix& theMatrix) noexcept : matrix{theMatrix} { };

            [[nodiscard]] size_t Dimension() const noexcept { return matrix.Dimension(); }

             /**
             * Provides access to square matrix of symbols.
             */
            const auto& operator()() const noexcept {
                return (*(matrix.sym_exp_matrix));
            }

            /**
             * Provides access to an element from within the matrix of symbols.
             */
            const auto& operator()(SquareMatrix<Monomial>::IndexView index) const noexcept(!debug_mode) {
                return (*(matrix.sym_exp_matrix))(index);
            }

            /**
             * Convenience method, provide access to element in matrix.
             */
            inline const auto& operator()(const size_t col, const size_t row) const noexcept(!debug_mode) {
                return (*(matrix.sym_exp_matrix))(SquareMatrix<Monomial>::Index{col, row});
            }
        } SymbolMatrix;


    protected:
        /** Matrix, as symbolic expression */
        std::unique_ptr<SquareMatrix<Monomial>> sym_exp_matrix;

        /** Global prefactor linking operator matrix to monomial matrix. */
        std::complex<double> global_prefactor;

    public:
        /** Construct precomputed monomial matrix without operator matrix. */
        MonomialMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                       std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix,
                       bool is_hermitian, std::complex<double> prefactor = std::complex<double>{1.0, 0.0});

        /** Compute monomial matrix from operator matrix, registering new symbols as necessary. */
        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix);

        /**
         * Compute monomial matrix from operator matrix, registering new symbols as necessary, and multiplying all
         * elements by a global factor. */
        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix,
                       std::complex<double> prefactor);

        /** Construct monomial matrix, with pre-calculated Monomials AND operator matrix */
        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix,
                       std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix);

        /** Destruct monomial matrix. */
        ~MonomialMatrix() noexcept;

        /**
         * Force renumbering of matrix bases keys
         */
        void renumerate_bases(const SymbolTable& symbols, double zero_tolerance) final;

        /**
         * Gets pointer to raw data
         */
        const Monomial* raw_data() const noexcept {
            return this->sym_exp_matrix->raw();
        }

        /**
         * Pre-multiply by a Monomial.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        pre_multiply(const Monomial& lhs, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const override;

        /**
         * Post-multiply by a Monomial.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        post_multiply(const Monomial& rhs, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const override;

        /**
         * Pre-multiply by a Polynomial.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        pre_multiply(const Polynomial& lhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const override;

        /**
         * Post-multiply by a Polynomial.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        post_multiply(const Polynomial& rhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const override;

        /**
         * Get global prefactor relating OperatorMatrix to actual monomials within.
         */
        [[nodiscard]] constexpr std::complex<double> global_factor() const noexcept {
            return this->global_prefactor;
        }

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

    template<>
    struct MatrixSpecialization<Monomial> { using type = MonomialMatrix; };
}