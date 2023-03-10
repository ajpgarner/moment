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
    public:
        const Context& context;
        const size_t fundamental_dimension;

    private:
        std::vector<Representation> representations;

    public:
        Group(const Context& context, Representation&& basis_rep);

    public:
        /**
         * Generate all elements of group from a set of generators using Dimino's algorithm.
         * @param generators Generator matrices, all square matrices of same dimension.
         * @return List of group elements.
         */
        static std::vector<repmat_t>
        dimino_generation(const std::vector<repmat_t>& generators,
                         size_t max_subgroup_size = 1000000);


    };

}