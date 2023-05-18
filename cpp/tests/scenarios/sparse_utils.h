/**
 * sparse_utils.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

#include <complex>
#include <initializer_list>
#include <vector>

#include "utilities/float_utils.h"



namespace Moment::Tests {
    template<typename value_t = double>
    inline Eigen::SparseMatrix<value_t> sparse_id(size_t dim) {
        Eigen::SparseMatrix<value_t> id(static_cast<int>(dim), static_cast<int>(dim));
        id.setIdentity();
        return id;
    }

    template<typename value_t = double>
    inline Eigen::SparseMatrix<value_t> make_sparse(size_t dim, std::initializer_list<value_t> vals) {
        using Index_t = typename Eigen::SparseMatrix<value_t>::Index;

        std::vector<Eigen::Triplet<value_t>> trips;

        trips.reserve(vals.size());
        assert(vals.size() == dim * dim);
        auto iter = vals.begin();
        for (Index_t i = 0; i < dim; ++i) {
            for (Index_t j = 0; j < dim; ++j) {
                value_t val = *iter;
                if (!approximately_zero(val)) {
                    trips.emplace_back(i, j, val);
                }
                ++iter;
            }
        }

        Eigen::SparseMatrix<value_t> output(static_cast<Index_t>(dim), static_cast<Index_t>(dim));
        output.setFromTriplets(trips.begin(), trips.end());

        return output;
    }

    template<typename value_t = double>
    inline Eigen::SparseMatrix<value_t> make_sparse_vector(std::initializer_list<value_t> values) {
        using Index_t = typename Eigen::SparseVector<value_t>::Index;

        const auto dim = static_cast<int>(values.size());
        const size_t nnz = std::transform_reduce(values.begin(), values.end(), 0ULL, std::plus{},
                                                 [](value_t val) -> size_t {
                                                        return approximately_zero(val) ? 1ULL : 0ULL;
                                                 });

        Eigen::SparseVector<value_t> sparse(static_cast<Index_t>(dim));
        sparse.reserve(static_cast<Index_t>(nnz));

        auto iter = values.begin();
        for (Eigen::Index i = 0; i < dim; ++i) {
            value_t val = *iter;
            if (!approximately_zero(val)) {
                sparse.insert(i) = val;
            }
            ++iter;
        }
        sparse.finalize();

        return sparse;
    }

    inline Eigen::SparseMatrix<double> one_elem(size_t dim, size_t i, size_t j) {
        Eigen::SparseMatrix<double> matrix(static_cast<Eigen::Index>(dim), static_cast<Eigen::Index>(dim));
        matrix.insert(static_cast<Eigen::Index>(i), static_cast<Eigen::Index>(j)) = 1.0;
        matrix.makeCompressed();
        return matrix;
    }

}