/**
 * group.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "representation.h"

#include <Eigen/Sparse>

#include <vector>

namespace Moment {

    class Context;

    class Group {
        std::vector<Representation> representations;

    public:
        Group(const std::vector<Eigen::SparseMatrix<double>>& generators);

    public:
        /**
         * Generate all elements of group from a set of generators using Dimino's algorithm.
         * @param generators Generator matrices, all square matrices of same dimension.
         * @return List of group elements.
         */
        static std::vector<Eigen::SparseMatrix<double>>
        dimino_generation(const std::vector<Eigen::SparseMatrix<double>>& generators,
                         size_t max_subgroup_size = 1000000);

    };

}