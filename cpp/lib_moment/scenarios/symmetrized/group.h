/**
 * group.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "representation.h"

#include <Eigen/Sparse>

#include <mutex>
#include <shared_mutex>
#include <vector>

namespace Moment {
    class Context;

    namespace errors {
        /**
         * Error issued when something fails when adding a symmetry.
         */
        class bad_symmetry : public std::runtime_error {
        public:
            explicit bad_symmetry(const std::string& what) : std::runtime_error{what} { }
        };
    }
};


namespace Moment::Symmetrized {

    class Group {
    public:
        /** Context this group represents a symmetry on */
        const Context& context;

        /** Matrix dimension of the fundamental representation of the group */
        const size_t fundamental_dimension;

        /** Number of unique group elements */
        const size_t size;

    private:
        mutable std::shared_mutex mutex;
        std::vector<std::unique_ptr<Representation>> representations;

    public:
        Group(const Context& context, std::unique_ptr<Representation> basis_rep);

        const Representation& create_representation(size_t word_length);

        const Representation& representation(size_t word_length) const;

    private:


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