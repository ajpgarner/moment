/**
 * matrix_basis.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "matrix_basis.h"
#include "matrix.h"

namespace Moment {
  namespace {
        template<typename BasisInfo>
        typename BasisInfo::CellularType::GetBasisType
        getDenseCellular(const MatrixBasis::MatrixBasisImpl<BasisInfo>& impl);

        template<>
        DenseMonolithicBasisInfo::CellularType::GetBasisType
        getDenseCellular<DenseMonolithicBasisInfo>(
                const MatrixBasis::MatrixBasisImpl<DenseMonolithicBasisInfo>& impl) {
            return impl.basis.Dense();
        }

        template<>
        DenseMonolithicComplexBasisInfo::CellularType::GetBasisType
        getDenseCellular<DenseMonolithicComplexBasisInfo>(
                const MatrixBasis::MatrixBasisImpl<DenseMonolithicComplexBasisInfo>& impl) {
            return impl.basis.DenseComplex();
        }

        template<typename BasisInfo>
        typename BasisInfo::CellularType::GetBasisType
        getSparseCellular(const MatrixBasis::MatrixBasisImpl<BasisInfo>& impl);

        template<>
        SparseMonolithicBasisInfo::CellularType::GetBasisType
        getSparseCellular<SparseMonolithicBasisInfo>(
                const MatrixBasis::MatrixBasisImpl<SparseMonolithicBasisInfo>& impl) {
            return impl.basis.Sparse();
        }

        template<>
        SparseMonolithicComplexBasisInfo::CellularType::GetBasisType
        getSparseCellular<SparseMonolithicComplexBasisInfo>(
                const MatrixBasis::MatrixBasisImpl<SparseMonolithicComplexBasisInfo>& impl) {
            return impl.basis.SparseComplex();
        }

        /** Infer dense monolithic basis */
        template<typename BasisInfo>
        typename BasisInfo::MakeStorageType
        do_infer_dense_monolithic(const MatrixBasis::MatrixBasisImpl<BasisInfo>& impl) {
            using Index_t = typename BasisInfo::RealMatrixType::Index;

            // Ensures dense basis exists (creates otherwise)
            auto [dense_re, dense_im] = getDenseCellular<BasisInfo>(impl);

            // Common
            auto dim = static_cast<Index_t>(impl.basis.matrix.Dimension());
            Index_t length = dim * dim;

            // Real
            auto re_height = static_cast<Index_t>(dense_re.size());
            auto re_ptr = std::make_unique<typename BasisInfo::RealMatrixType>(length, re_height);
            auto& real = *re_ptr;
            Index_t re_col_index = 0;
            for (const auto& basis_elem : dense_re) {
                assert(basis_elem.outerSize() * basis_elem.innerSize() == length);
                real.col(re_col_index) = basis_elem.reshaped();
                ++re_col_index;
            }

            // Imaginary
            auto im_height = static_cast<Index_t>(dense_im.size());
            auto im_ptr = std::make_unique<typename BasisInfo::ImMatrixType>(length, im_height);
            auto& imag = *im_ptr;
            Index_t im_col_index = 0;
            for (const auto& basis_elem : dense_im) {
                assert(basis_elem.outerSize() * basis_elem.innerSize() == length);
                imag.col(im_col_index) = basis_elem.reshaped();
                ++im_col_index;
            }

            return {std::move(re_ptr), std::move(im_ptr)};
        }

        /** Infer sparse monolithic basis */
        template<typename BasisInfo>
        typename BasisInfo::MakeStorageType
        do_infer_sparse_monolithic(const MatrixBasis::MatrixBasisImpl<BasisInfo>& impl) {
            // Ensures sparse basis exists (creates otherwise)
            auto [sparse_re, sparse_im] = getSparseCellular<BasisInfo>(impl);

            // Common
            auto dim = static_cast<typename BasisInfo::IndexType>(impl.basis.matrix.Dimension());
            typename BasisInfo::IndexType flat_dim = dim * dim;

            // Make real
            size_t re_nnz = 0;
            for (const auto& b : sparse_re) {
                re_nnz += b.nonZeros();
            }
            auto re_height = static_cast<typename BasisInfo::IndexType>(sparse_re.size());


            // Prepare real trips
            std::vector<Eigen::Triplet<typename BasisInfo::RealMatrixType::Scalar>> re_trips;
            re_trips.reserve(re_nnz);
            typename BasisInfo::IndexType re_col_index = 0;
            for (const auto& src : sparse_re) {

                for (int src_col=0; src_col < src.outerSize(); ++src_col) {
                    for (typename BasisInfo::RealMatrixType::InnerIterator it(src, src_col); it; ++it) {
                        typename BasisInfo::IndexType remapped_index = (src_col*dim) + it.row();
                        assert(remapped_index < flat_dim);
                        re_trips.emplace_back(remapped_index, re_col_index, it.value());
                    }
                }
                ++re_col_index;
            }

            // Copy to sparse matrix
            auto sparse_mono_re = std::make_unique<typename BasisInfo::RealMatrixType>(flat_dim, re_height );
            sparse_mono_re->setFromSortedTriplets(re_trips.cbegin(), re_trips.cend());

            // Make imaginary
            size_t im_nnz = 0;
            for (const auto& b : sparse_im) {
                im_nnz += b.nonZeros();
            }
            auto im_height = static_cast<typename BasisInfo::IndexType>(sparse_im.size());

            // Prepare trips
            std::vector<Eigen::Triplet<typename BasisInfo::ImMatrixType::Scalar>> im_trips;
            im_trips.reserve(im_nnz);
            typename BasisInfo::IndexType im_col_index = 0;
            for (const auto& src : sparse_im) {

                for (int src_col=0; src_col < src.outerSize(); ++src_col) {
                    for (typename BasisInfo::ImMatrixType::InnerIterator it(src, src_col); it; ++it) {
                        typename BasisInfo::IndexType remapped_index = (src_col*dim) + it.row();
                        assert(remapped_index < flat_dim);
                        im_trips.emplace_back(remapped_index, im_col_index,  it.value());
                    }
                }
                ++im_col_index;
            }

            // Copy to sparse matrix
            auto sparse_mono_im = std::make_unique<typename BasisInfo::ImMatrixType>(flat_dim, im_height);
            sparse_mono_im->setFromSortedTriplets(im_trips.cbegin(), im_trips.cend());

            // And return
            return {std::move(sparse_mono_re), std::move(sparse_mono_im)};
        }
    }

    MatrixBasis::MatrixBasis(const Matrix &rhs_matrix, MatrixBasis &&rhs) noexcept :
            matrix{rhs_matrix},
            Dense{*this, std::move(rhs.Dense)},
            DenseComplex{*this, std::move(rhs.DenseComplex)},
            Sparse{*this, std::move(rhs.Sparse)},
            SparseComplex{*this, std::move(rhs.SparseComplex)},
            DenseMonolithic{*this, std::move(rhs.DenseMonolithic)},
            DenseMonolithicComplex{*this, std::move(rhs.DenseMonolithicComplex)},
            SparseMonolithic{*this, std::move(rhs.SparseMonolithic)},
            SparseMonolithicComplex{*this, std::move(rhs.SparseMonolithicComplex)} {
    }

    DenseBasisInfo::MakeStorageType MatrixBasis::create_dense() {
        if (!this->matrix.real_coefficients()) {
            throw errors::bad_basis_error{"Matrix has complex coefficients, and so a basis of type [R,C] cannot be created."};
        }
        return this->matrix.create_dense_basis();
    }

    SparseBasisInfo::MakeStorageType MatrixBasis::create_sparse() {
        if (!this->matrix.real_coefficients()) {
            throw errors::bad_basis_error{"Matrix has complex coefficients, and so a basis of type [R,C] cannot be created."};
        }
        return this->matrix.create_sparse_basis();
    }

    DenseComplexBasisInfo::MakeStorageType MatrixBasis::create_dense_complex() {
        return this->matrix.create_dense_complex_basis();
    }

    SparseComplexBasisInfo::MakeStorageType MatrixBasis::create_sparse_complex() {
        return this->matrix.create_sparse_complex_basis();
    }

    template<>
    inline DenseBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseBasisInfo>::create_basis() const {
        return this->basis.create_dense();
    }

    template<>
    inline DenseComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseComplexBasisInfo>::create_basis() const {
        return this->basis.create_dense_complex();
    }

    template<>
    inline SparseBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseBasisInfo>::create_basis() const {
        return this->basis.create_sparse();
    }

    template<>
    inline SparseComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseComplexBasisInfo>::create_basis() const {
        return this->basis.create_sparse_complex();
    }


    /** Infer dense, real, monolithic basis */
    template<>
    DenseMonolithicBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseMonolithicBasisInfo>::create_basis() const {
        return do_infer_dense_monolithic<DenseMonolithicBasisInfo>(*this);
    }

    /** Infer dense, complex, monolithic basis */
    template<>
    DenseMonolithicComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseMonolithicComplexBasisInfo>::create_basis() const {
        return do_infer_dense_monolithic<DenseMonolithicComplexBasisInfo>(*this);
    }

    template<>
    SparseMonolithicBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseMonolithicBasisInfo>::create_basis() const {
        return do_infer_sparse_monolithic<SparseMonolithicBasisInfo>(*this);
    }


    template<>
    SparseMonolithicComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseMonolithicComplexBasisInfo>::create_basis() const {
        return do_infer_sparse_monolithic<SparseMonolithicComplexBasisInfo>(*this);
    }

}