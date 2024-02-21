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

    class OperatorSequence;
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
            inline const auto& operator()(const size_t row, const size_t col) const noexcept(!debug_mode) {
                return (*(matrix.sym_exp_matrix))(SquareMatrix<Monomial>::Index{row, col});
            }
        } SymbolMatrix;


    protected:
        /** Matrix, as symbolic expression */
        std::unique_ptr<SquareMatrix<Monomial>> sym_exp_matrix;

        /** Global prefactor linking operator matrix to monomial matrix. */
        std::complex<double> global_prefactor;

    public:
        /** Construct monomial matrix, with pre-calculated Monomials AND operator matrix */
        MonomialMatrix(SymbolTable& symbols,
                       std::unique_ptr<OperatorMatrix> operator_matrix,
                       std::unique_ptr<OperatorMatrix> aliased_operator_matrix,
                       std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix);

        /** Construct precomputed monomial matrix without operator matrix. */
        MonomialMatrix(const Context& context, SymbolTable& symbols, double zero_tolerance,
                       std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix,
                       bool is_hermitian, std::complex<double> prefactor = std::complex<double>{1.0, 0.0});

        /**
         * Compute monomial matrix from operator matrix, registering new symbols as necessary (in single-threaded
         * manner).
         */
        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix);

        /**
         * Compute monomial matrix from operator matrix, registering new symbols as necessary (in single-threaded
         * manner), and multiplying all elements by a global factor.
         */
        MonomialMatrix(SymbolTable& symbols, std::unique_ptr<OperatorMatrix> operator_matrix,
                       std::complex<double> prefactor);

        /**
         * Compute monomial matrix from operator matrix, registering new symbols (in single-threaded manner).
         * Aliasing specialization.
         */
        MonomialMatrix(SymbolTable& symbols,
                       std::unique_ptr<OperatorMatrix> unaliased_matrix,
                       std::unique_ptr<OperatorMatrix> aliased_matrix);

        /**
         * Compute monomial matrix from operator matrix, registering new symbols as necessary (in single-threaded
         * manner), and multiplying all elements by a global factor.
         * Aliasing specialization.
         */
        MonomialMatrix(SymbolTable& symbols,
                       std::unique_ptr<OperatorMatrix> unaliased_matrix,
                       std::unique_ptr<OperatorMatrix> aliased_matrix,
                       std::complex<double> prefactor);

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

        using SymbolicMatrix::pre_multiply;

        using SymbolicMatrix::post_multiply;


        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        pre_multiply(const OperatorSequence& lhs, std::complex<double> weight,
                     const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const override;

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        post_multiply(const OperatorSequence& lhs, std::complex<double> weight,
                      const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const override;

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        pre_multiply(const RawPolynomial& lhs,
                     const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const override;

        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        post_multiply(const RawPolynomial& rhs,
                      const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const override;

        using SymbolicMatrix::add;

        std::unique_ptr<PolynomialMatrix> add(const Monomial& rhs, const PolynomialFactory& poly_factory,
                                              Multithreading::MultiThreadPolicy policy) const override;

        std::unique_ptr<PolynomialMatrix> add(const Polynomial& rhs, const PolynomialFactory& poly_factory,
                                              Multithreading::MultiThreadPolicy policy) const override;

        /**
         * Get global prefactor relating OperatorMatrix to actual monomials within.
         */
        [[nodiscard]] constexpr std::complex<double> global_factor() const noexcept {
            return this->global_prefactor;
        }

    protected:

        [[nodiscard]] DenseBasisInfo::MakeStorageType create_dense_basis() const override;

        [[nodiscard]] SparseBasisInfo::MakeStorageType create_sparse_basis() const override;

        [[nodiscard]] DenseComplexBasisInfo::MakeStorageType create_dense_complex_basis() const override;

        [[nodiscard]] SparseComplexBasisInfo::MakeStorageType create_sparse_complex_basis() const override;

        /**
         * Look up basis elements in matrix
         */
        void identify_symbols_and_basis_indices();

    public:
       /**
        * Constructs a matrix of all zeros.
        * @param context The context, for the (zero) operator sequences, if the context defines operators.
        * @param symbol_table The associated symbol table.
        * @param dimension The dimension of the matrix.
        * @return Owning pointer to a newly constructed monomial matrix.
        */
        static std::unique_ptr<MonomialMatrix>
        zero_matrix(const Context& context, SymbolTable& symbol_table, size_t dimension);

        std::unique_ptr<SymbolicMatrix> clone(Multithreading::MultiThreadPolicy policy) const override;

    };

    template<>
    struct MatrixSpecialization<Monomial> { using type = MonomialMatrix; };
}