/**
 * multithreading.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <iosfwd>
#include <limits>
#include <string>

#include "integer_types.h"

namespace Moment::Multithreading {

    enum class MultiThreadPolicy : int {
        Never = -1,
        Optional = 0,
        Always = 1
    };

    /** Output MT policy. */
    std::ostream& operator<<(std::ostream&, MultiThreadPolicy policy);

    /** Get MT policy as a string. */
    std::string to_string(MultiThreadPolicy policy);


    /** Set this value to something other than max() to hard-cap the number of worker threads that may be created. */
    constexpr const size_t worker_thread_limit = std::numeric_limits<size_t>::max();

    /** The minimum number of elements in a requested matrix to trigger multi-threaded creation in optional mode. */
    constexpr const size_t minimum_matrix_element_count = 6400; // = 80 x 80 matrix, or larger.

    /** The minimum number of elements in a requested matrix to trigger multi-threaded multiplication in optional mode.*/
    constexpr const size_t minimum_matrix_multiply_element_count = 6400; // = 80 x 80 matrix, or larger.

    /** The minimum product of of elements in a requested matrix with log2 of the number of rules,
     * to trigger multi-threaded creation in optional mode. */
    constexpr const size_t minimum_rule_difficulty = 6400; // = 80 x 80 matrix with 1 rule, or harder.

    /** The minimum number of possible elements in an OSG to trigger multi-threaded creation in optional mode. */
    constexpr const size_t minimum_osg_element_count = 1000;

    /** Threshold, for multi-threaded group  representation creation: raw dimension * raw dimension * group elems. */
    constexpr const size_t minimum_group_rep_difficulty = 5000; // e.g. one ~25*25 matrix with 8 group elements.


    /**
     * Query the maximum number of workers that may be created
     * (This is the smaller value of the number of physical cores recorded and the hard-coded maximum, if any.
     */
    [[nodiscard]] size_t get_max_worker_threads();

    /**
     * Should we multithread a task? Generic multithread three-way switch, resolving 'optional' case on difficulty.
     * @param policy The multithreading policy.
     * @param optional_threshold The triggering threshold, above which multithreading is used.
     * @param actual The actual task difficulty estimate, to compare against optional_threshold. */
    [[nodiscard]] inline constexpr bool should_multithread(MultiThreadPolicy policy,
                                                           size_t optional_threshold, size_t actual) noexcept {
        switch(policy) {
            case MultiThreadPolicy::Never:
                return false;
            case MultiThreadPolicy::Always:
                return true;
            default:
            case MultiThreadPolicy::Optional:
                return actual >= optional_threshold;
        }
    }

    /**
     * Should the matrix creation be multithreaded?
     */
    [[nodiscard]] bool should_multithread_matrix_creation(MultiThreadPolicy policy, size_t elements) noexcept;

    /**
     * Should the rule application be multithreaded?
     */
    [[nodiscard]] bool should_multithread_rule_application(MultiThreadPolicy policy,
                                                           size_t elements, size_t rules) noexcept;

    /**
     * Should the rule application be multithreaded?
     */
    [[nodiscard]] bool should_multithread_group_rep_generation(MultiThreadPolicy policy,
                                                               size_t raw_dim, size_t group_elements) noexcept;

    /**
     * Should the operator sequence generation be multithreaded?
     * (NB: Currently not implemented!)
     */
    [[nodiscard]] bool should_multithread_osg(MultiThreadPolicy policy, size_t potential_elements) noexcept;

}