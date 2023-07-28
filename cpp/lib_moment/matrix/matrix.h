/**
 * matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "matrix_basis.h"
#include "matrix_basis_type.h"

#include <cassert>

#include <complex>
#include <iosfwd>
#include <map>
#include <memory>
#include <set>
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
        std::unique_ptr<OperatorMatrix> op_mat;

    public:
        friend class MatrixBasis;
        friend class OperatorMatrix;

        /**
         * Numeric basis for this matrix, in terms of real and imaginary parts of symbols.
         */
        MatrixBasis Basis;


    public:
        Matrix(const Context& context, SymbolTable& symbols, size_t dimension = 0);

        Matrix(const Matrix& rhs) = delete;

        Matrix(Matrix&& rhs) noexcept = delete;

        virtual ~Matrix() noexcept;

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
         [[nodiscard]] bool has_operator_matrix() const noexcept {
             return static_cast<bool>(this->op_mat);
         }

         /**
          * Gets operator matrix.
          * @throws errors::missing_component if no operator matrix defined for this matrix.
          */
         [[nodiscard]] const OperatorMatrix& operator_matrix() const;


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
          friend std::ostream& operator<<(std::ostream& os, const Matrix& matrix);

        /**
         * Force renumbering of matrix bases keys
         */
        virtual void renumerate_bases(const SymbolTable& symbols, double zero_tolerance) = 0;

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
    struct MatrixSpecialization { using type = Matrix; };

}