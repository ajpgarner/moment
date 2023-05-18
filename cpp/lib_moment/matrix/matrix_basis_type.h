/**
 * matrix_basis_type.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/Dense>
#include <Eigen/SparseCore>

#include <memory>
#include <utility>
#include <vector>

namespace Moment {

    template<typename real_t, typename im_t>
    struct MatrixBasis_Elems {
        using RealMatrixType = real_t;
        using ImMatrixType =  im_t;
        using IndexType = typename RealMatrixType::Index;

        /** True, if real basis matrix type is same as complex basis matrix type. */
        static constexpr bool ComplexComplex = std::is_same_v<real_t, im_t>;
    };

    template<typename real_t, typename im_t, typename re_storage_t, typename im_storage_t>
    struct MatrixBasis_Storage : public MatrixBasis_Elems<real_t, im_t> {
        using RealStorageType = re_storage_t;
        using ImStorageType = im_storage_t;
        using MakeStorageType = std::pair<RealStorageType, ImStorageType>;
    };

    template<typename real_t, typename im_t>
    struct MatrixBasis_CellularStorage
            : public MatrixBasis_Storage<real_t, im_t, std::vector<real_t>, std::vector<im_t>> {
        using typename MatrixBasis_Storage<real_t, im_t, std::vector<real_t>, std::vector<im_t>>::RealStorageType;
        using typename MatrixBasis_Storage<real_t, im_t, std::vector<real_t>, std::vector<im_t>>::ImStorageType;
        using GetBasisType = std::pair<const RealStorageType&,
                                       const ImStorageType&>;
        static constexpr bool Monolithic = false;

        constexpr static GetBasisType GetBasis(const RealStorageType& re, const ImStorageType& im) noexcept {
            return {re, im};
        }

    };

    template<typename real_t, typename im_t>
    struct MatrixBasis_MonolithicStorage
            : public MatrixBasis_Storage<real_t, im_t, std::unique_ptr<real_t>, std::unique_ptr<im_t>> {
        using typename MatrixBasis_Storage<real_t, im_t, std::unique_ptr<real_t>, std::unique_ptr<im_t>>::RealStorageType;
        using typename MatrixBasis_Storage<real_t, im_t, std::unique_ptr<real_t>, std::unique_ptr<im_t>>::ImStorageType;
        using typename MatrixBasis_Storage<real_t, im_t, std::unique_ptr<real_t>, std::unique_ptr<im_t>>::RealMatrixType;
        using typename MatrixBasis_Storage<real_t, im_t, std::unique_ptr<real_t>, std::unique_ptr<im_t>>::ImMatrixType;

        using GetBasisType = std::pair<const RealMatrixType&, const ImMatrixType&>;

        static constexpr bool Monolithic = true;

        constexpr static GetBasisType GetBasis(const RealStorageType& re, const ImStorageType& im) noexcept {
            return {*re, *im};
        }
    };

    /**
     * Type-helper for matrix bases.
     * @tparam is_sparse True if matrices are sparse, false for dense.
     * @tparam is_monolith True if singular monolithic matrix, false for 'cellular' array.
     * @tparam entirely_complex True if real term factors can also be complex.
     */
    template<bool is_sparse, bool is_monolith, bool entirely_complex>
    struct MatrixBasisType;

    /** Dense, cellular, [R,C] */
    template<>
    struct MatrixBasisType<false, false, false>
        : public MatrixBasis_CellularStorage<Eigen::MatrixXd, Eigen::MatrixXcd> { };


    /** Sparse, cellular, [R,C] */
    template<>
    struct MatrixBasisType<true, false, false>
        : public MatrixBasis_CellularStorage<Eigen::SparseMatrix<double>, Eigen::SparseMatrix<std::complex<double>>> {
        using RealTripletType = Eigen::Triplet<double>;
        using ImTripletType = Eigen::Triplet<std::complex<double>>;
    };

    /** Dense, monolithic, [R,C] */
    template<>
    struct MatrixBasisType<false, true, false>
        : public MatrixBasis_MonolithicStorage<Eigen::MatrixXd, Eigen::MatrixXcd> {
        using CellularType = MatrixBasisType<false, false, false>;
    };


    /** Sparse, monolithic, [R,C] */
    template<>
    struct MatrixBasisType<true, true, false>
        : public MatrixBasis_MonolithicStorage<Eigen::SparseMatrix<double>, Eigen::SparseMatrix<std::complex<double>>> {
        using CellularType = MatrixBasisType<true, false, false>;
        using RealTripletType = Eigen::Triplet<double>;
        using ImTripletType = Eigen::Triplet<std::complex<double>>;
    };

    /** Dense, cellular, [C,C] */
    template<>
    struct MatrixBasisType<false, false, true>
            : public MatrixBasis_CellularStorage<Eigen::MatrixXcd, Eigen::MatrixXcd> {

    };

    /** Sparse, cellular, [C,C] */
    template<>
    struct MatrixBasisType<true, false, true>
        : public MatrixBasis_CellularStorage<Eigen::SparseMatrix<std::complex<double>>,
                                             Eigen::SparseMatrix<std::complex<double>>> {
        using RealTripletType = Eigen::Triplet<std::complex<double>>;
        using ImTripletType = Eigen::Triplet<std::complex<double>>;
    };

    /** Dense, monolithic, [C,C] */
    template<>
    struct MatrixBasisType<false, true, true>
        : public MatrixBasis_MonolithicStorage<Eigen::MatrixXcd, Eigen::MatrixXcd> {
        using CellularType = MatrixBasisType<false, false, true>;
    };


    /** Sparse, monolithic, [C,C] */
    template<>
    struct MatrixBasisType<true, true, true>
        : public MatrixBasis_MonolithicStorage<Eigen::SparseMatrix<std::complex<double>>,
                                               Eigen::SparseMatrix<std::complex<double>>> {
        using CellularType = MatrixBasisType<true, false, true>;
        using RealTripletType = Eigen::Triplet<std::complex<double>>;
        using ImTripletType = Eigen::Triplet<std::complex<double>>;
    };

    /** Alias: Dense, Cellular, [R, C] */
    using DenseBasisInfo = MatrixBasisType<false, false, false>;
    static_assert(!DenseBasisInfo::Monolithic);
    static_assert(!DenseBasisInfo::ComplexComplex);

    /** Alias: Sparse, Cellular, [R, C] */
    using SparseBasisInfo = MatrixBasisType<true, false, false>;
    static_assert(!SparseBasisInfo::Monolithic);
    static_assert(!SparseBasisInfo::ComplexComplex);

    /** Alias: Dense, Monolithic, [R, C] */
    using DenseMonolithicBasisInfo = MatrixBasisType<false, true, false>;
    static_assert(DenseMonolithicBasisInfo::Monolithic);
    static_assert(!DenseMonolithicBasisInfo::ComplexComplex);

    /**  Alias:Sparse, Monolithic, [R, C] */
    using SparseMonolithicBasisInfo = MatrixBasisType<true, true, false>;
    static_assert(SparseMonolithicBasisInfo::Monolithic);
    static_assert(!SparseMonolithicBasisInfo::ComplexComplex);

    /**  Alias:Dense, Cellular, [C, C] */
    using DenseComplexBasisInfo = MatrixBasisType<false, false, true>;
    static_assert(!DenseComplexBasisInfo::Monolithic);
    static_assert(DenseComplexBasisInfo::ComplexComplex);

    /**  Alias: Sparse, Cellular, [C, C] */
    using SparseComplexBasisInfo = MatrixBasisType<true, false, true>;
    static_assert(!SparseComplexBasisInfo::Monolithic);
    static_assert(SparseComplexBasisInfo::ComplexComplex);

    /**  Alias: Dense, Monolithic, [C, C] */
    using DenseMonolithicComplexBasisInfo = MatrixBasisType<false, true, true>;
    static_assert(DenseMonolithicComplexBasisInfo::Monolithic);
    static_assert(DenseMonolithicComplexBasisInfo::ComplexComplex);

    /** Alias: Sparse, Monolithic, [C, C] */
    using SparseMonolithicComplexBasisInfo = MatrixBasisType<true, true, true>;
    static_assert(SparseMonolithicComplexBasisInfo::Monolithic);
    static_assert(SparseMonolithicComplexBasisInfo::ComplexComplex);

}