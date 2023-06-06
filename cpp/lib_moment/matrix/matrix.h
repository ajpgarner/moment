/**
 * matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "properties/matrix_properties.h"

#include "matrix_basis.h"
#include "matrix_basis_type.h"

#include <cassert>

#include <complex>
#include <memory>
#include <string>
#include <vector>

namespace Moment {

    class OperatorMatrix;
    class SymbolTable;
    class Context;

    class Matrix {

    public:
        /** Defining scenario for matrix (especially: rules for simplifying operator sequences). */
        const Context& context;

    protected:
        /** Look-up key for symbols */
        SymbolTable& symbol_table;

        /** Square matrix size */
        size_t dimension = 0;

        /** Symbol matrix properties (basis size, etc.) */
        std::unique_ptr<MatrixProperties> mat_prop;

        /** Operator matrix, if set - (may be null) */
        std::unique_ptr<OperatorMatrix> op_mat;

    public:
        /**
         * Table of symbols for entire system.
         */
        const SymbolTable& Symbols;


    public:
        friend class MatrixBasis;

        /**
         * Numeric basis for this matrix, in terms of real and imaginary parts of symbols.
         */
        MatrixBasis Basis;


    public:
        Matrix(const Context& context, SymbolTable& symbols, size_t dimension = 0);

        Matrix(Matrix&& rhs) noexcept;

        virtual ~Matrix() noexcept;

        /**
         * Dimension of the matrix
         */
        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        /**
         * Description of matrix type
         */
        [[nodiscard]] const std::string& description() const {
            assert(this->mat_prop);
            return this->mat_prop->Description();
        }

        /**
         * True, if matrix is Hermitian.
         */
        [[nodiscard]] bool is_hermitian() const noexcept {
            assert(this->mat_prop);
            return this->mat_prop->IsHermitian();
        }

        /**
         * Properties of the matrix.
         */
        [[nodiscard]] const MatrixProperties& SMP() const noexcept {
            assert(this->mat_prop);
            return *this->mat_prop;
        }

        /**
         * True if matrix has operator matrix.
         */
         [[nodiscard]] bool has_operator_matrix() const noexcept {
             return static_cast<bool>(this->op_mat);
         }

         /**
          * Gets operator matrix.
          * @throws errors::missing_component if no operator matrix defined for this matrix.
          */
         [[nodiscard]] const OperatorMatrix& operator_matrix() const;

         /**
          * Test if every co-efficient before symbols in this matrix is real.
          */
         [[nodiscard]] virtual bool real_coefficients() const noexcept = 0;

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
         * Force renumbering of matrix bases keys
         */
        virtual void renumerate_bases(const SymbolTable& symbols) = 0;

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

        void set_description(std::string new_description) noexcept;


    };

}