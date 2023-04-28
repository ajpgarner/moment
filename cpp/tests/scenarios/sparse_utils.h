/**
 * sparse_utils.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

#include "utilities/float_utils.h"

namespace Moment::Tests {
    inline Eigen::SparseMatrix<double> sparse_id(size_t dim) {
        Eigen::SparseMatrix<double> id(static_cast<int>(dim), static_cast<int>(dim));
        id.setIdentity();
        return id;
    }

    inline Eigen::SparseMatrix<double> make_sparse(size_t dim, std::initializer_list<double> vals) {
        std::vector <Eigen::Triplet<double>> trips;
        trips.reserve(vals.size());
        assert(vals.size() == dim * dim);
        auto iter = vals.begin();
        for (int i = 0; i < dim; ++i) {
            for (int j = 0; j < dim; ++j) {
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
        const size_t nnz = std::transform_reduce(values.begin(), values.end(), 0,
                                                 std::plus{},
                                                 [](double val) -> size_t { return (val != 0.0) ? 1 : 0; });

        Eigen::SparseVector<double> sparse(static_cast<int>(dim));
        sparse.reserve(static_cast<Eigen::SparseVector<double>::Index>(nnz));

        auto iter = values.begin();
        for (int i = 0; i < dim; ++i) {
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
        Eigen::SparseMatrix<double> matrix(static_cast<int>(dim), static_cast<int>(dim));
        matrix.insert(static_cast<int>(i), static_cast<int>(j)) = 1.0;
        matrix.makeCompressed();
        return matrix;
    }

}