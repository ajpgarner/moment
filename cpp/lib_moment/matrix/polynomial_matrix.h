/**
 * polynomial_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once
#include "symbolic_matrix.h"

#include "symbolic/polynomial.h"
#include "tensor/square_matrix.h"

#include <memory>
#include <span>

namespace Moment {

    class MonomialMatrix;
    class PolynomialFactory;

    class PolynomialMatrix : public SymbolicMatrix {
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
            * Gets a polynomial from within the square matrix.
            */
            const Polynomial& operator()(SquareMatrix<Polynomial>::IndexView index) const noexcept(!debug_mode) {
                return (*(matrix.sym_exp_matrix))(index);
            };

            /**
             * Convenience method, provide access to element in matrix.
             */
            inline const auto& operator()(const size_t col, const size_t row) const noexcept(!debug_mode) {
                return (*(matrix.sym_exp_matrix))(SquareMatrix<Polynomial>::Index{col, row});
            }

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

        PolynomialMatrix(const Context& context, const PolynomialFactory& factory, SymbolTable& symbols,
                         std::span<const MonomialMatrix*> constituents);

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


        using SymbolicMatrix::pre_multiply;

        using SymbolicMatrix::post_multiply;

        std::unique_ptr<SymbolicMatrix> pre_multiply(const OperatorSequence& lhs, std::complex<double> weight,
                                                     const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                                                     Multithreading::MultiThreadPolicy policy) const override {
            throw errors::cannot_multiply_exception{"PolynomialMatrix::pre_multiply not implemented."};
        }

        std::unique_ptr<SymbolicMatrix> post_multiply(const OperatorSequence& rhs, std::complex<double> weight,
                                                      const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                                                      Multithreading::MultiThreadPolicy policy) const override {
            throw errors::cannot_multiply_exception{"PolynomialMatrix::post_multiply not implemented."};
        }

        std::unique_ptr<SymbolicMatrix> pre_multiply(const RawPolynomial& lhs,
                                                     const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                                                     Multithreading::MultiThreadPolicy policy) const override {
            throw errors::cannot_multiply_exception{"PolynomialMatrix::pre_multiply not implemented."};
        }

        std::unique_ptr<SymbolicMatrix> post_multiply(const RawPolynomial& rhs,
                                                      const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                                                      Multithreading::MultiThreadPolicy policy) const override {
            throw errors::cannot_multiply_exception{"PolynomialMatrix::post_multiply not implemented."};
        }

        using SymbolicMatrix::add;

        std::unique_ptr<PolynomialMatrix> add(const Monomial& rhs, const PolynomialFactory& poly_factory,
                                              Multithreading::MultiThreadPolicy policy) const override;

        std::unique_ptr<PolynomialMatrix> add(const Polynomial& rhs, const PolynomialFactory& poly_factory,
                                              Multithreading::MultiThreadPolicy policy) const override;

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

    template<>
    struct MatrixSpecialization<Polynomial> { using type = PolynomialMatrix; };
}