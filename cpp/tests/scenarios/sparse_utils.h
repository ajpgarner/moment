/**
 * sparse_utils.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

#include <vector>

#include "utilities/float_utils.h"



namespace Moment::Tests {
    inline Eigen::SparseMatrix<double> sparse_id(size_t dim) {
        Eigen::SparseMatrix<double> id(static_cast<int>(dim), static_cast<int>(dim));
        id.setIdentity();
        return id;
    }

    inline Eigen::SparseMatrix<double> make_sparse(size_t dim, std::initializer_list<double> vals) {
        using Index_t = Eigen::SparseMatrix<double>::StorageIndex;

        std::vector<Eigen::Triplet<double>> trips;

        trips.reserve(vals.size());
        assert(vals.size() == dim * dim);
        auto iter = vals.begin();
        for (Index_t i = 0; i < dim; ++i) {
            for (Index_t j = 0; j < dim; ++j) {
                double val = *iter;
                if (val != 0) {
                    trips.emplace_back(i, j, val);
                }
                ++iter;
            }
        }

        Eigen::SparseMatrix<double> sparse(static_cast<int>(dim), static_cast<int>(dim));
        sparse.setFromTriplets(trips.begin(), trips.end());

        return sparse;
    }

    inline Eigen::SparseMatrix<double> make_sparse_vector( std::initializer_list<double> values) {
        const auto dim = static_cast<int>(values.size());
        const size_t nnz = std::transform_reduce(values.begin(), values.end(), 0ULL,
                                                 std::plus{},
                                                 [](double val) -> size_t { return (val != 0.0) ? 1ULL : 0ULL; });

        Eigen::SparseVector<double> sparse(static_cast<Eigen::Index>(dim));
        sparse.reserve(static_cast<Eigen::Index>(nnz));

        auto iter = values.begin();
        for (Eigen::Index i = 0; i < dim; ++i) {
            double val = *iter;
            if (val != 0) {
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