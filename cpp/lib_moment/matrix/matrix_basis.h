/**
 * matrix_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "matrix_basis_type.h"

#include <atomic>
#include <mutex>
#include <stdexcept>
#include <string>

namespace Moment {
    class SymbolicMatrix;

    namespace errors {
        class bad_basis_error : public std::runtime_error {
        public:
            explicit bad_basis_error(const std::string& what) : std::runtime_error{what} { }
        };
    }

    /**
     * Bases for matrix.
     * Implements deferred creation and caching.
     */
    class MatrixBasis {
    public:

        friend class SymbolicMatrix;

        template<typename MBTInfo>
        struct MatrixBasisImpl {
        public:
            MatrixBasis& basis;

        private:
            mutable std::atomic<bool> done = false;
            mutable typename MBTInfo::RealStorageType re;
            mutable typename MBTInfo::ImStorageType im;

        public:
            explicit MatrixBasisImpl(MatrixBasis& basis) : basis{basis} { }

            MatrixBasisImpl(MatrixBasis& basis, MatrixBasisImpl&& rhs)
                    : basis{basis}, re{std::move(rhs.re)}, im{std::move(rhs.im)} {
                this->done = rhs.done.load(std::memory_order_relaxed);
            }

            [[nodiscard]] bool is_done() const noexcept {
                return this->done.load(std::memory_order_acquire);
            }

            /** Gets the basis, from cache if possible, otherwise, create it. */
            [[nodiscard]] typename MBTInfo::GetBasisType operator()() const {
                if (this->done.load(std::memory_order_acquire)) {
                    return MBTInfo::GetBasis(this->re, this->im);
                }

                // Otherwise lock and try again:
                std::unique_lock write_lock{this->basis.mutex};
                if (this->done.load(std::memory_order_acquire)) {
                    return MBTInfo::GetBasis(this->re, this->im); // unlock
                }

                // Create and flag as done
                auto [re_part, im_part] = this->create_basis();
                this->re = std::move(re_part);
                this->im = std::move(im_part);
                this->done.store(true, std::memory_order_release);

                // Return references to created parts
                return MBTInfo::GetBasis(this->re, this->im); // unlock~
            }

            /** Constructs the basis from scratch */
            [[nodiscard]] typename MBTInfo::MakeStorageType create_basis() const;
        };

    public:
        const SymbolicMatrix& matrix;

        /** Dense basis, indexed by symbols, with no imaginary elements for real part of symbols. */
        MatrixBasisImpl<DenseBasisInfo> Dense;

        /** Dense basis, indexed by symbols, with imaginary elements for real part of symbols. */
        MatrixBasisImpl<DenseComplexBasisInfo>  DenseComplex;

        /** Sparse basis, indexed by symbols, with no imaginary elements for real part of symbols. */
        MatrixBasisImpl<SparseBasisInfo>  Sparse;

        /** Sparse basis, indexed by symbols, with imaginary elements for real part of symbols. */
        MatrixBasisImpl<SparseComplexBasisInfo>   SparseComplex;

        /**
         * Dense monolithic basis (to be reshaped), with no imaginary elements for real part of symbols.
         * Each column represents a matrix element; each row represents a basis element.
         * Matrix elements (i.e. col index) are given in col-major ordering.
         */
        MatrixBasisImpl<DenseMonolithicBasisInfo>  DenseMonolithic;

        /**
         * Dense monolithic basis (to be reshaped), with imaginary elements for real part of symbols.
         * Each column represents a matrix element; each row represents a basis element.
         * Matrix elements (i.e. col index) are given in col-major ordering.
         */
        MatrixBasisImpl<DenseMonolithicComplexBasisInfo>   DenseMonolithicComplex;

        /**
         * Sparse monolithic basis (to be reshaped), with no imaginary elements for real part of symbols.
         * Each column represents a matrix element; each row represents a basis element.
         * Matrix elements (i.e. col index) are given in col-major ordering.
         */
        MatrixBasisImpl<SparseMonolithicBasisInfo>   SparseMonolithic;

        /**
         * Sparse monolithic basis (to be reshaped), with imaginary elements for real part of symbols.
         * Each column represents a matrix element; each row represents a basis element.
         * Matrix elements (i.e. col index) are given in col-major ordering.
         */
        MatrixBasisImpl<SparseMonolithicComplexBasisInfo>    SparseMonolithicComplex;

    private:
        mutable std::recursive_mutex mutex;

    public:
        explicit MatrixBasis(const SymbolicMatrix& matrix) : matrix{matrix},
                                                             Dense{*this}, DenseComplex{*this},
                                                             Sparse{*this}, SparseComplex{*this},
                                                             DenseMonolithic{*this}, DenseMonolithicComplex{*this},
                                                             SparseMonolithic{*this}, SparseMonolithicComplex{*this} { }

        MatrixBasis(const SymbolicMatrix& matrix, MatrixBasis&& rhs) noexcept;

    private:
        DenseBasisInfo::MakeStorageType create_dense();
        SparseBasisInfo::MakeStorageType create_sparse();
        DenseComplexBasisInfo::MakeStorageType create_dense_complex();
        SparseComplexBasisInfo::MakeStorageType create_sparse_complex();
    };

    template<>
    DenseBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseBasisInfo>::create_basis() const;

    template<>
    DenseComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseComplexBasisInfo>::create_basis() const;

    template<>
    SparseBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseBasisInfo>::create_basis() const;

    template<>
    SparseComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseComplexBasisInfo>::create_basis() const;

    template<>
    DenseMonolithicBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseMonolithicBasisInfo>::create_basis() const;

    template<>
    DenseMonolithicComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseMonolithicComplexBasisInfo>::create_basis() const;

    template<>
    SparseMonolithicBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseMonolithicBasisInfo>::create_basis() const;

    template<>
    SparseMonolithicComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseMonolithicComplexBasisInfo>::create_basis() const;




}