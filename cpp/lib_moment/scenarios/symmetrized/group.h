/**
 * group.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "representation.h"

#include "multithreading/multithreading.h"

#include "multithreading/maintains_mutex.h"
#include "utilities/small_vector.h"

#include <Eigen/Sparse>

#include <memory>
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

    class RepresentationMapper;

    class Group : private MaintainsMutex {
    public:
        using build_list_t = SmallVector<size_t, 4>;

    public:
        /** Context this group represents a symmetry on */
        const Context& context;

        /** Matrix dimension of the fundamental representation of the group */
        const size_t fundamental_dimension;

        /** Number of unique group elements */
        const size_t size;

    private:
        std::vector<std::unique_ptr<Representation>> representations;
        std::vector<std::unique_ptr<RepresentationMapper>> mappers;

    public:
        Group(const Context& context, std::unique_ptr<Representation> basis_rep);

        ~Group() noexcept;

        const Representation&
        create_representation(size_t word_length,
                              Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional);

        const Representation& representation(size_t word_length) const;

    private:



    public:
        /**
         * Generate all elements of group from a set of generators using Dimino's algorithm.
         * @param generators Generator matrices, all square matrices of same dimension.
         * @return List of group elements.
         */
        [[nodiscard]] static std::vector<repmat_t>
        dimino_generation(const std::vector<repmat_t>& generators,
                         size_t max_subgroup_size = 1000000);

        [[nodiscard]] static build_list_t decompose_build_list(size_t word_length);

        /**
         * For a given word length, get its parent representations.
         * Undefined behaviour if representations are not in supplied vector.
         */
        [[nodiscard]] static std::pair<const Representation&, const Representation&>
        determine_parent_representations(const std::vector<std::unique_ptr<Representation>>&, size_t wl) noexcept;

    private:
        void identify_and_build_representations(const size_t word_length, Multithreading::MultiThreadPolicy mt_policy);

    };

}