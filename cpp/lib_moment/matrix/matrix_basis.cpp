/**
 * matrix_basis.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "matrix_basis.h"
#include "symbolic_matrix.h"

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

            // Ensures dense basis exists (create it otherwise).
            auto [dense_re, dense_im] = getDenseCellular<BasisInfo>(impl);

            // Common
            auto dim = static_cast<Index_t>(impl.basis.matrix.Dimension());
            const Index_t flat_dim = dim * dim;

            // Real
            const auto re_height = static_cast<Index_t>(dense_re.size());
            auto re_ptr = std::make_unique<typename BasisInfo::RealMatrixType>(re_height, flat_dim);
            auto& real = *re_ptr;
            Index_t re_basis_elem_index = 0;
            for (const auto& basis_elem : dense_re) {
                assert(basis_elem.outerSize() * basis_elem.innerSize() == flat_dim);
                real.row(re_basis_elem_index) = basis_elem.reshaped();
                ++re_basis_elem_index;
            }

            // Imaginary
            const auto im_height = static_cast<Index_t>(dense_im.size());
            auto im_ptr = std::make_unique<typename BasisInfo::ImMatrixType>(im_height, flat_dim);
            auto& imag = *im_ptr;
            Index_t im_basis_elem_index = 0;
            for (const auto& basis_elem : dense_im) {
                assert(basis_elem.outerSize() * basis_elem.innerSize() == flat_dim);
                imag.row(im_basis_elem_index) = basis_elem.reshaped();
                ++im_basis_elem_index;
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
            const auto re_height = static_cast<typename BasisInfo::IndexType>(sparse_re.size());

            // Prepare real trips
            std::vector<Eigen::Triplet<typename BasisInfo::RealMatrixType::Scalar>> re_trips;
            re_trips.reserve(re_nnz);
            typename BasisInfo::IndexType re_col_index = 0;
            for (const auto& src : sparse_re) {
                for (int src_col=0; src_col < src.outerSize(); ++src_col) {
                    for (typename BasisInfo::RealMatrixType::InnerIterator it(src, src_col); it; ++it) {
                        typename BasisInfo::IndexType remapped_index = (src_col*dim) + it.row();
                        assert(remapped_index < flat_dim);
                        re_trips.emplace_back(re_col_index, remapped_index, it.value());
                    }
                }
                ++re_col_index;
            }

            // Copy to sparse matrix
            auto sparse_mono_re = std::make_unique<typename BasisInfo::RealMatrixType>(re_height, flat_dim);
            sparse_mono_re->setFromTriplets(re_trips.cbegin(), re_trips.cend());

            // Make imaginary
            size_t im_nnz = 0;
            for (const auto& b : sparse_im) {
                im_nnz += b.nonZeros();
            }
            const auto im_height = static_cast<typename BasisInfo::IndexType>(sparse_im.size());

            // Prepare imaginary trips
            std::vector<Eigen::Triplet<typename BasisInfo::ImMatrixType::Scalar>> im_trips;
            im_trips.reserve(im_nnz);
            typename BasisInfo::IndexType im_col_index = 0;
            for (const auto& src : sparse_im) {

                for (int src_col=0; src_col < src.outerSize(); ++src_col) {
                    for (typename BasisInfo::ImMatrixType::InnerIterator it(src, src_col); it; ++it) {
                        typename BasisInfo::IndexType remapped_index = (src_col*dim) + it.row();
                        assert(remapped_index < flat_dim);
                        im_trips.emplace_back(im_col_index, remapped_index, it.value());
                    }
                }
                ++im_col_index;
            }

            // Copy to sparse matrix
            auto sparse_mono_im = std::make_unique<typename BasisInfo::ImMatrixType>(im_height, flat_dim);
            sparse_mono_im->setFromTriplets(im_trips.cbegin(), im_trips.cend());

            // And return
            return {std::move(sparse_mono_re), std::move(sparse_mono_im)};
        }
    }

    MatrixBasis::MatrixBasis(const SymbolicMatrix &rhs_matrix, MatrixBasis &&rhs) noexcept :
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
        if (this->matrix.HasComplexCoefficients()) {
            throw errors::bad_basis_error{"Matrix has complex coefficients, and so a basis of type [R,C] cannot be created."};
        }
        return this->matrix.create_dense_basis();
    }

    SparseBasisInfo::MakeStorageType MatrixBasis::create_sparse() {
        if (this->matrix.HasComplexCoefficients()) {
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
    DenseBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseBasisInfo>::create_basis() const {
        return this->basis.create_dense();
    }

    template<>
    DenseComplexBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<DenseComplexBasisInfo>::create_basis() const {
        return this->basis.create_dense_complex();
    }

    template<>
    SparseBasisInfo::MakeStorageType
    MatrixBasis::MatrixBasisImpl<SparseBasisInfo>::create_basis() const {
        return this->basis.create_sparse();
    }

    template<>
    SparseComplexBasisInfo::MakeStorageType
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