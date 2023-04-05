/**
 * matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "matrix.h"


namespace Moment {

    Matrix::MatrixBasis::MatrixBasis(const Matrix &rhs_matrix, Matrix::MatrixBasis &&rhs) noexcept :
        matrix{rhs_matrix},
        dense_re(std::move(rhs.dense_re)),
        dense_im(std::move(rhs.dense_im)),
        dense_mono_re(std::move(rhs.dense_mono_re)),
        dense_mono_im(std::move(rhs.dense_mono_im)),
        sparse_re(std::move(rhs.sparse_re)),
        sparse_im(std::move(rhs.sparse_im)),
        sparse_mono_re(std::move(rhs.sparse_mono_re)),
        sparse_mono_im(std::move(rhs.sparse_mono_im)) {

        // While under construction, only one thread can access  object:¬
        this->done_dense = rhs.done_dense.load(std::memory_order_relaxed);
        this->done_dense_mono = rhs.done_dense_mono.load(std::memory_order_relaxed);
        this->done_sparse = rhs.done_sparse.load(std::memory_order_relaxed);
        this->done_sparse_mono = rhs.done_sparse_mono.load(std::memory_order_relaxed);
    }

    Matrix::MatrixBasis::dense_basis_indexed_ref_t Matrix::MatrixBasis::Dense() const {
        // Already exists:
        if (this->done_dense.load(std::memory_order_acquire)) {
            return {this->dense_re, this->dense_im};
        }

        // Otherwise lock
        std::unique_lock write_lock{this->mutex};
        if (this->done_dense.load(std::memory_order_acquire)) {
            return {this->dense_re, this->dense_im}; // unlock
        }

        // Create and flag as done
        auto [re, im] = this->matrix.create_dense_basis();
        this->dense_re = std::move(re);
        this->dense_im = std::move(im);
        this->done_dense.store(true, std::memory_order_release);

        // Return refs
        return {this->dense_re, this->dense_im}; // unlock
    }

    Matrix::MatrixBasis::dense_basis_mono_ref_t Matrix::MatrixBasis::DenseMonolithic() const {
        // Already exists:
        if (this->done_dense_mono.load(std::memory_order_acquire)) {
            return {*this->dense_mono_re, *this->dense_mono_im};
        }

        // Otherwise upgrade lock
        std::unique_lock write_lock{this->mutex};
        if (this->done_dense_mono.load(std::memory_order_acquire)) {
            return {*this->dense_mono_re, *this->dense_mono_im};  // unlock
        }

        // Ensure dense basis is created
        if (!this->done_dense) {
            auto [re, im] = this->matrix.create_dense_basis();
            this->dense_re = std::move(re);
            this->dense_im = std::move(im);
            this->done_dense = true;
        }

        // Create:
        this->infer_dense_monolithic();
        assert(this->done_dense_mono);
        return {*this->dense_mono_re, *this->dense_mono_im}; // unlock
    }

    Matrix::MatrixBasis::sparse_basis_indexed_ref_t Matrix::MatrixBasis::Sparse() const {
        // Already exists:
        if (this->done_sparse.load(std::memory_order_acquire)) {
            return {this->sparse_re, this->sparse_im};
        }

        // Otherwise lock
        std::unique_lock write_lock{this->mutex};
        if (this->done_sparse.load(std::memory_order_acquire)) {
            return {this->sparse_re, this->sparse_im}; // unlock
        }

        // Create:
        auto [re, im] = this->matrix.create_sparse_basis();
        this->sparse_re = std::move(re);
        this->sparse_im = std::move(im);
        this->done_sparse.store(true, std::memory_order_release);
        return {this->sparse_re, this->sparse_im}; // unlock
    }

    Matrix::MatrixBasis::sparse_basis_mono_ref_t Matrix::MatrixBasis::SparseMonolithic() const {
       // Already exists:
       if (this->done_sparse_mono.load(std::memory_order_acquire)) {
           return {*this->sparse_mono_re, *this->sparse_mono_im};
       }

       // Otherwise upgrade lock
       std::unique_lock write_lock{this->mutex};
       if (this->done_sparse_mono.load(std::memory_order_acquire)) {
           return {*this->sparse_mono_re, *this->sparse_mono_im}; // unlock
       }

       // Ensure sparse basis is first created
       if (!this->done_sparse) {
           auto [re, im] = this->matrix.create_sparse_basis();
           this->sparse_re = std::move(re);
           this->sparse_im = std::move(im);
           this->done_sparse = true;
       }

       // Create:
       this->infer_sparse_monolithic();
       assert(this->done_sparse_mono);
       return {*this->sparse_mono_re, *this->sparse_mono_im}; // unlock
    }


    void Matrix::MatrixBasis::infer_dense_monolithic() const {
        assert(this->done_dense);

        // Common
        auto dim = static_cast<dense_real_elem_t::Index>(this->matrix.dimension);
        dense_real_elem_t::Index length = dim * dim;

        // Real
        auto re_height = static_cast<dense_real_elem_t::Index>(this->dense_re.size());
        this->dense_mono_re = std::make_unique<dense_real_elem_t>(length, re_height);
        auto& real = *this->dense_mono_re;
        dense_real_elem_t::Index re_col_index = 0;
        for (const auto& basis_elem : this->dense_re) {
            assert(basis_elem.outerSize() * basis_elem.innerSize() == length);
            real.col(re_col_index) = basis_elem.reshaped();
            ++re_col_index;
        }

        // Imaginary
        auto im_height = static_cast<dense_real_elem_t::Index>(this->dense_im.size());
        this->dense_mono_im = std::make_unique<dense_complex_elem_t>(length, im_height);
        auto& im = *this->dense_mono_im;
        dense_real_elem_t::Index im_col_index = 0;
        for (const auto& basis_elem : this->dense_im) {
            assert(basis_elem.outerSize() * basis_elem.innerSize() == length);
            im.col(im_col_index) = basis_elem.reshaped();
            ++im_col_index;
        }

        this->done_dense_mono.store(true, std::memory_order_release);
    }

    void Matrix::MatrixBasis::infer_sparse_monolithic() const {
        assert(this->done_sparse);

        // Common
        auto dim = static_cast<sparse_real_elem_t::Index>(this->matrix.dimension);
        sparse_real_elem_t::Index flat_dim = dim * dim;

        // Make real
        size_t re_nnz = 0;
        for (const auto& b : this->sparse_re) {
            re_nnz += b.nonZeros();
        }
        auto re_height = static_cast<sparse_real_elem_t::Index>(this->sparse_re.size());


        // Prepare real trips
        std::vector<Eigen::Triplet<sparse_real_elem_t::Scalar>> re_trips;
        re_trips.reserve(re_nnz);
        sparse_real_elem_t::Index re_col_index = 0;
        for (const auto& src : this->sparse_re) {

            for (int src_col=0; src_col < src.outerSize(); ++src_col) {
                for (sparse_real_elem_t::InnerIterator it(src, src_col); it; ++it) {
                    sparse_real_elem_t::Index remapped_index = (src_col*dim) + it.row();
                    assert(remapped_index < flat_dim);
                    re_trips.emplace_back(remapped_index, re_col_index, it.value());
                }
            }
            ++re_col_index;
        }

        // Copy to sparse matrix
        this->sparse_mono_re = std::make_unique<sparse_real_elem_t>(flat_dim, re_height );
        this->sparse_mono_re->setFromSortedTriplets(re_trips.cbegin(), re_trips.cend());

        // Make imaginary
        size_t im_nnz = 0;
        for (const auto& b : this->sparse_im) {
            im_nnz += b.nonZeros();
        }
        auto im_height = static_cast<sparse_complex_elem_t::Index>(this->sparse_im.size());


        // Prepare trips
        std::vector<Eigen::Triplet<sparse_complex_elem_t::Scalar>> im_trips;
        im_trips.reserve(im_nnz);
        sparse_complex_elem_t::Index im_col_index = 0;
        for (const auto& src : this->sparse_im) {

            for (int src_col=0; src_col < src.outerSize(); ++src_col) {
                for (sparse_complex_elem_t::InnerIterator it(src, src_col); it; ++it) {
                    sparse_complex_elem_t::Index remapped_index = (src_col*dim) + it.row();
                    assert(remapped_index < flat_dim);
                    im_trips.emplace_back(remapped_index, im_col_index,  it.value());
                }
            }
            ++im_col_index;
        }

        // Copy to sparse matrix
        this->sparse_mono_im = std::make_unique<sparse_complex_elem_t>(flat_dim, im_height);
        this->sparse_mono_im->setFromSortedTriplets(im_trips.cbegin(), im_trips.cend());

        this->done_sparse_mono.store(true, std::memory_order_release);
    }
}