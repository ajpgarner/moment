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
#include <mutex>
#include <vector>
#include <memory>

namespace Moment {

    class SymbolTable;
    class Context;

    /**
     * Representation of a basis element, and monolithic variants thereof.
     */
    using dense_basis_elem_t = Eigen::Matrix<double, Eigen::Dynamic, Eigen::Dynamic, Eigen::RowMajor>;

    /**
     * An array of dense basis elements.
     */
    using dense_basis_storage_t = std::vector<dense_basis_elem_t>;

    /**
     * Pair of references to dense basis arrays.
     */
    using dense_basis_indexed_ref_t = std::pair<const dense_basis_storage_t&, const dense_basis_storage_t&>;

    /**
     * Pair of references to monolithic dense basis matrices.
     */
    using dense_basis_mono_ref_t = std::pair<const dense_basis_elem_t&, const dense_basis_elem_t&>;



    /**
     * Sparse representation of a basis element, and monolithic variants thereof.
     */
    using sparse_basis_elem_t = Eigen::SparseMatrix<double>;

    /**
     * An array of sparse basis elements.
     */
    using sparse_basis_storage_t = std::vector<sparse_basis_elem_t>;

    /**
     * Pair of references to sparse basis arrays.
     */
    using sparse_basis_indexed_ref_t = std::pair<const sparse_basis_storage_t&, const sparse_basis_storage_t&>;

    /**
     * Pair of references to monolithic sparse basis matrices.
     */
    using sparse_basis_mono_ref_t = std::pair<const sparse_basis_elem_t&, const sparse_basis_elem_t&>;


    class Matrix {

    public:
        /** Defining scenario for matrix (especially: rules for simplifying operator sequences). */
        const Context& context;

    protected:
        /** Look-up key for symbols */
        SymbolTable& symbol_table;

        /** Square matrix size */
        size_t dimension = 0;

        /** True, if Hermitian */
        bool is_hermitian = false;

        /** Symbol matrix properties (basis size, etc.) */
        std::unique_ptr<MatrixProperties> mat_prop;

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
            const Matrix& matrix;

        private:
            mutable std::mutex mutex;

            mutable std::atomic<bool> done_dense = false;
            mutable dense_basis_storage_t dense_re;
            mutable dense_basis_storage_t dense_im;

            mutable std::atomic<bool> done_dense_mono = false;
            mutable std::unique_ptr<dense_basis_elem_t> dense_mono_re;
            mutable std::unique_ptr<dense_basis_elem_t> dense_mono_im;

            mutable std::atomic<bool> done_sparse = false;
            mutable sparse_basis_storage_t sparse_re;
            mutable sparse_basis_storage_t sparse_im;

            mutable std::atomic<bool> done_sparse_mono = false;
            mutable std::unique_ptr<sparse_basis_elem_t> sparse_mono_re;
            mutable std::unique_ptr<sparse_basis_elem_t> sparse_mono_im;

        public:
            MatrixBasis(const Matrix& matrix) : matrix{matrix} { }

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

        explicit Matrix(const Context& context, SymbolTable& symbols, size_t dimension = 0)
            : context{context}, symbol_table{symbols}, Symbols{symbols}, dimension{dimension}, Basis{*this} { }

        Matrix(Matrix&& rhs) noexcept : context{rhs.context}, symbol_table{rhs.symbol_table}, dimension{rhs.dimension},
                      is_hermitian{rhs.is_hermitian}, mat_prop{std::move(rhs.mat_prop)}, Symbols{rhs.Symbols},
                      Basis{*this, std::move(rhs.Basis)} { }

        virtual ~Matrix() noexcept = default;

        /**
         * Description of matrix type
         */
        [[nodiscard]] virtual std::string description() const = 0;


        /**
         * Dimension of the matrix
         */
        [[nodiscard]] constexpr size_t Dimension() const noexcept {
            return this->dimension;
        }

        /**
         * True, if matrix is Hermitian
         */
        [[nodiscard]] constexpr bool IsHermitian() const noexcept {
            return this->is_hermitian;
        }

        /**
         * Properties of the matrix
         */
        [[nodiscard]] const MatrixProperties& SMP() const noexcept {
            assert(this->mat_prop);
            return *this->mat_prop;
        }


    protected:
        /**
         * Create dense basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         */
        [[nodiscard]] virtual std::pair<dense_basis_storage_t, dense_basis_storage_t>
        create_dense_basis() const = 0;

        /**
         * Create sparse basis.
         * For thread safety, call read lock on hosting matrix system.
         * basis_mutex is assumed to be uniquely locked when this function is entered.
         */
        [[nodiscard]] virtual std::pair<sparse_basis_storage_t, sparse_basis_storage_t>
        create_sparse_basis() const = 0;


    };

}