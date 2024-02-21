/**
 * symbolic_matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "matrix_basis.h"
#include "matrix_basis_type.h"

#include "multithreading/multithreading.h"

#include <cassert>

#include <complex>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
#include <string>
#include <vector>

namespace Moment {

    class Context;
    struct Monomial;
    class OperatorMatrix;
    class OperatorSequence;
    class Polynomial;
    class PolynomialFactory;
    class PolynomialMatrix;
    class SymbolTable;
    class RawPolynomial;

    namespace errors {
        /**
         * Exception to throw if cloning is not possible for some reason.
         */
        class cannot_clone_exception : public std::logic_error {
        public:
            explicit cannot_clone_exception(const std::string& what)
                    : std::logic_error{what} { }
        };

        /**
         * Exception to throw if multiplication is not possible for some reason.
         */
        class cannot_multiply_exception : public std::logic_error {
        public:
            explicit cannot_multiply_exception(const std::string& what)
                    : std::logic_error{what} { }
        };

        /**
         * Exception to throw if addition is not possible for some reason.
         */
        class cannot_add_exception : public  std::logic_error {
        public:
            explicit cannot_add_exception(const std::string& what)
                    : std::logic_error{what} { }
        };
    };

    class SymbolicMatrix {

    public:
        /** Defining scenario for matrix (especially: rules for simplifying operator sequences). */
        const Context& context;

        /** Table of symbols for entire system. */
        const SymbolTable& symbols;

    protected:
        /** Table of symbols for entire system. */
        SymbolTable& symbol_table;

        /** Square matrix size */
        size_t dimension = 0;

        /** Matrix is Hermitian? */
        bool hermitian = false;

        /** True if matrix has any complex coefficients in front of its elements (real or otherwise) */
        bool complex_coefficients = false;

        /** True if matrix could generate moments that take complex values (e.g. from non-Hermitian operators). */
        bool complex_basis = false;

        /** Human-readable name for matrix */
        std::string description;

        /** Symbols mentioned in the matrix. */
        std::set<symbol_name_t> included_symbols;

        /** Included real-valued basis elements, corresponding to real parts of symbols. */
        std::set<symbol_name_t> real_basis_elements;

        /** Included real-valued basis elements, corresponding to imaginary parts of symbols. */
        std::set<symbol_name_t> imaginary_basis_elements;

        /** Map from included symbols IDs to basis indices. */
        std::map<symbol_name_t, std::pair<ptrdiff_t, ptrdiff_t>> basis_key;

        /** Operator matrix, if set - (may be null) */
        std::unique_ptr<OperatorMatrix> unaliased_op_mat;

        /** Aliased operator matrix, if set - (may be null) */
        std::unique_ptr<OperatorMatrix> aliased_op_mat;

    public:
        friend class MatrixBasis;
        friend class OperatorMatrix;

        /**
         * Numeric basis for this matrix, in terms of real and imaginary parts of symbols.
         */
        MatrixBasis Basis;


    public:
        SymbolicMatrix(const Context& context, SymbolTable& symbols, size_t dimension = 0);

        SymbolicMatrix(const SymbolicMatrix& rhs) = delete;

        SymbolicMatrix(SymbolicMatrix&& rhs) noexcept = delete;

        virtual ~SymbolicMatrix() noexcept;

        /**
         * Dimension of the matrix
         */
        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        /** Short description of matrix type. */
        [[nodiscard]] const std::string& Description() const {
            return this->description;
        }

        /** True, if matrix is Hermitian. */
        [[nodiscard]] bool Hermitian() const noexcept {
            return this->hermitian;
        }

        /**
         * True, if one or more imaginary parts of the SDP basis are be required to correctly specify the matrix.
         * That is, if the 'b' sdpvars would have an impact on the matrix.
         * Note, this could be true, and the matrix real, e.g. because of terms like "i<X>" where <X> is anti-hermitian.
         */
        [[nodiscard]] bool HasComplexBasis() const noexcept {
            return this->complex_basis;
        }

        /** True, if any coefficients within the matrix are complex. */
        [[nodiscard]] bool HasComplexCoefficients() const noexcept {
            return this->complex_coefficients;
        }

        /** Set of all symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& IncludedSymbols() const noexcept {
            return this->included_symbols;
        }

        /** Set of real symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& RealBasisIndices() const noexcept {
            return this->real_basis_elements;
        }

        /** Set of imaginary symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& ImaginaryBasisIndices() const noexcept {
            return this->imaginary_basis_elements;
        }

        /** Set of imaginary symbols involved in this matrix. */
        [[nodiscard]] constexpr const auto& BasisKey() const noexcept {
            return this->basis_key;
        }

        /**  True if matrix has operator matrix. */
        [[nodiscard]] bool has_unaliased_operator_matrix() const noexcept {
            return static_cast<bool>(this->unaliased_op_mat);
        }

        /**  True if matrix has aliased operator matrix (or there is no aliasing). */
         [[nodiscard]] bool has_aliased_operator_matrix() const noexcept;

         /**
          * Gets unaliased operator matrix.
          * Operator sequences should be interpreted as operators.
          * @throws errors::missing_component if no operator matrix defined for this matrix.
          */
         [[nodiscard]] const OperatorMatrix& unaliased_operator_matrix() const;

         /**
          * Gets operator matrix, with any aliasing (if applicable).
          * Operator sequences should be interpreted as moments.
          * @throws errors::missing_component if no operator matrix defined for this matrix.
          */
         [[nodiscard]] const OperatorMatrix& aliased_operator_matrix() const;

         /**
          * True if matrix is defined in terms of monomial symbols.
          */
         [[nodiscard]] virtual bool is_monomial() const noexcept {
              return true;
          }

          /**
           * True if matrix is defined in terms of polynomial symbols.
           */
         [[nodiscard]] inline bool is_polynomial() const noexcept {
             return !this->is_monomial();
         }

         /**
          * Output matrix properties.
          */
          friend std::ostream& operator<<(std::ostream& os, const SymbolicMatrix& matrix);

        /**
         * Force renumbering of matrix bases keys
         */
        virtual void renumerate_bases(const SymbolTable& symbols, double zero_tolerance) = 0;

        /**
         * Throws an error if this matrix cannot be multiplied for any reason.
         * @throws cannot_multiply_exception if some property of the matrix would prevent successful multiplication.
         */
        virtual void throw_error_if_cannot_multiply() const;

        /**
         * Create a new matrix by pre-multiplying this one by a weighted operator sequence.
         * @param lhs The operator sequence pre-factor.
         * @param weight A uniform weight to multiply the matrix by.
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] virtual std::unique_ptr<SymbolicMatrix>
        pre_multiply(const OperatorSequence& lhs, std::complex<double> weight,
                     const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by post-multiplying this one by a weighted operator sequence.
         * @param rhs The operator sequence post-factor.
         * @param weight A uniform weight to multiply the matrix by.
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] virtual std::unique_ptr<SymbolicMatrix>
        post_multiply(const OperatorSequence& rhs, std::complex<double> weight,
                      const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by pre-multiplying this one by a RawPolynomial.
         * @param rhs The raw polynomial pre-factor.
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] virtual std::unique_ptr<SymbolicMatrix>
        pre_multiply(const RawPolynomial& lhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by post-multiplying this one by a RawPolynomial.
         * @param rhs The raw polynomial post-factor.
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] virtual std::unique_ptr<SymbolicMatrix>
        post_multiply(const RawPolynomial& rhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by pre-multiplying this one by a Monomial.
         * @param lhs The monomial prefactor
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        pre_multiply(const Monomial& lhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by post-multiplying this one by a Monomial.
         * @param rhs The monomial post-factor
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        post_multiply(const Monomial& rhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by pre-multiplying this one by a Polynomial.
         * @param lhs The monomial pre-factor
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        pre_multiply(const Polynomial& lhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                     Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by post-multiplying this one by a Polynomial.
         * @param rhs The monomial post-factor
         * @param poly_factory Parameters for creating symbolic polynomials.
         * @param symbol_table Mutable symbol table: will register any new symbols after multiplication.
         * @param policy Attempt to multithread the multiplication?
         * @return Owning pointer to newly created matrix.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        post_multiply(const Polynomial& rhs, const PolynomialFactory& poly_factory, SymbolTable& symbol_table,
                      Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by adding a matrix this one.
         */
        [[nodiscard]] virtual std::unique_ptr<PolynomialMatrix>
        add(const SymbolicMatrix& rhs, const PolynomialFactory& poly_factory,
            Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by adding a monomial to this one.
         */
        [[nodiscard]] virtual std::unique_ptr<PolynomialMatrix>
        add(const Monomial& rhs, const PolynomialFactory& poly_factory,
            Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a new matrix by adding a polynomial to this one.
         */
        [[nodiscard]] virtual std::unique_ptr<PolynomialMatrix>
        add(const Polynomial& rhs, const PolynomialFactory& poly_factory,
            Multithreading::MultiThreadPolicy policy) const;

        /**
         * Create a copy of this matrix.
         * In general, this is expensive and should be avoided.
         */
        virtual std::unique_ptr<SymbolicMatrix> clone(Multithreading::MultiThreadPolicy policy) const;

    protected:
        void copy_properties_onto_clone(SymbolicMatrix& clone) const;

    protected:
        /**
         * Create dense basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         * @return Pair; first: basis for real part of symbols, second: basis for imaginary part of symbols.
         */
        [[nodiscard]] virtual DenseBasisInfo::MakeStorageType create_dense_basis() const = 0;

        /**
         * Create dense complex basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         * @return Pair; first: basis for real part of symbols, second: basis for imaginary part of symbols.
         */
        [[nodiscard]] virtual DenseComplexBasisInfo::MakeStorageType create_dense_complex_basis() const = 0;

        /**
         * Create sparse basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         * @return Pair; first: basis for real part of symbols, second: basis for imaginary part of symbols.
         */
        [[nodiscard]] virtual SparseBasisInfo::MakeStorageType create_sparse_basis() const = 0;

        /**
         * Create sparse basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         * @return Pair; first: basis for real part of symbols, second: basis for imaginary part of symbols.
         */
        [[nodiscard]] virtual SparseComplexBasisInfo::MakeStorageType create_sparse_complex_basis() const = 0;
    };

    template <typename elem_t>
    struct MatrixSpecialization { using type = SymbolicMatrix; };

}