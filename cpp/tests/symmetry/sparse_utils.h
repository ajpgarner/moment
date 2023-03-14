/**
 * sparse_utils.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <Eigen/Sparse>

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
}