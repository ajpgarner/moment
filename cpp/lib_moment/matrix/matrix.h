/**
 * matrix.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "matrix_properties.h"

#include <Eigen/Dense>
#include <Eigen/SparseCore>

#include <atomic>
#include <complex>
#include <memory>
#include <mutex>
#include <string>
#include <vector>

namespace Moment {

    class OperatorMatrix;
    class SymbolTable;
    class Context;

    /**
     * Representation of a real basis element, and monolithic variants thereof.
     */
    using dense_real_elem_t = Eigen::MatrixXd;
    static_assert(!dense_real_elem_t::IsRowMajor);

    /**
     * Representation of a complex basis element, and monolithic variants thereof.
     */
    using dense_complex_elem_t = Eigen::MatrixXcd;
    static_assert(!dense_complex_elem_t::IsRowMajor);

    /**
     * Sparse representation of a basis element, and monolithic variants thereof.
     */
    using sparse_real_elem_t = Eigen::SparseMatrix<double>;

    /**
     * Sparse representation of a complex basis element, and monolithic variants thereof.
     */
    using sparse_complex_elem_t = Eigen::SparseMatrix<std::complex<double>>;

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
        /**
         * Bases for matrix.
         * Implements deferred creation and caching.
         */
        class MatrixBasis {
        public:
            /**
              * An array of real dense basis elements.
              */
            using dense_real_storage_t = std::vector<dense_real_elem_t>;

            /**
             * An array of complex dense basis elements.
             */
            using dense_complex_storage_t = std::vector<dense_complex_elem_t>;

            /**
             * Pair of references to dense basis arrays.
             */
            using dense_basis_indexed_ref_t = std::pair<const dense_real_storage_t&, const dense_complex_storage_t&>;

            /**
             * Pair of references to monolithic dense basis matrices.
             */
            using dense_basis_mono_ref_t = std::pair<const dense_real_elem_t&, const dense_complex_elem_t&>;

            /**
             * An array of real sparse basis elements.
             */
            using sparse_real_storage_t = std::vector<sparse_real_elem_t>;

            /**
             * An array of complex sparse basis elements.
             */
            using sparse_complex_storage_t = std::vector<sparse_complex_elem_t>;

            /**
             * Pair of references to sparse basis arrays.
             */
            using sparse_basis_indexed_ref_t = std::pair<const sparse_real_storage_t&, const sparse_complex_storage_t&>;

            /**
             * Pair of references to monolithic sparse basis matrices.
             */
            using sparse_basis_mono_ref_t = std::pair<const sparse_real_elem_t&, const sparse_complex_elem_t&>;

        public:
            const Matrix& matrix;

        private:
            mutable std::mutex mutex;

            mutable std::atomic<bool> done_dense = false;
            mutable dense_real_storage_t dense_re;
            mutable dense_complex_storage_t dense_im;

            mutable std::atomic<bool> done_dense_mono = false;
            mutable std::unique_ptr<dense_real_elem_t> dense_mono_re;
            mutable std::unique_ptr<dense_complex_elem_t> dense_mono_im;

            mutable std::atomic<bool> done_sparse = false;
            mutable sparse_real_storage_t sparse_re;
            mutable sparse_complex_storage_t sparse_im;

            mutable std::atomic<bool> done_sparse_mono = false;
            mutable std::unique_ptr<sparse_real_elem_t> sparse_mono_re;
            mutable std::unique_ptr<sparse_complex_elem_t> sparse_mono_im;

        public:
            explicit MatrixBasis(const Matrix& matrix) : matrix{matrix} { }

            MatrixBasis(const Matrix& matrix, MatrixBasis&& rhs) noexcept;

            /**
             * Get dense basis, indexed by symbols.
             * Mutable function: creation is deferred until first request.
             * For thread safety, call read lock on hosting matrix system.
             * @return Pair of dense basis matrix vectors (symmetric, and anti-symmetric).
             */
            [[nodiscard]] dense_basis_indexed_ref_t Dense() const;

            /**
             * Get dense monolithic basis to be reshaped.
             * Mutable function: creation is deferred until first request.
             * For thread safety, call read lock on hosting matrix system.
             * @return Dense matrix, each column representing one basis element.
             */
            [[nodiscard]] dense_basis_mono_ref_t DenseMonolithic() const;

            /**
             * Get dense basis, indexed by symbols.
             * Mutable function: creation is deferred until first request.
             * For thread safety, call read lock on hosting matrix system.
             * @return Vector of sparse basis matrices, indexed by symbol id.
             */
            [[nodiscard]] sparse_basis_indexed_ref_t Sparse() const;

            /**
             * Get single "monolithic" sparse basis to be reshaped.
             * Mutable function: creation is deferred until first request.
             * For thread safety, call read lock on hosting matrix system.
             * @return Sparse matrix, each column representing one basis element.
             */
            [[nodiscard]] sparse_basis_mono_ref_t SparseMonolithic() const;

        private:
            void infer_dense_monolithic() const;

            void infer_sparse_monolithic() const;
        } Basis;


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
        [[nodiscard]] constexpr bool is_hermitian() const noexcept {
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
         */
        [[nodiscard]] virtual std::pair<MatrixBasis::dense_real_storage_t, MatrixBasis::dense_complex_storage_t>
        create_dense_basis() const = 0;

        /**
         * Create sparse basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         */
        [[nodiscard]] virtual std::pair<MatrixBasis::sparse_real_storage_t, MatrixBasis::sparse_complex_storage_t>
        create_sparse_basis() const = 0;

        void set_description(std::string new_description) noexcept;


    };

}