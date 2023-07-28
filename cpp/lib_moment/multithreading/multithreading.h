/**
 * multithreading.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <limits>

#include "integer_types.h"

namespace Moment::Multithreading {

    enum class MultiThreadPolicy : int {
        Never = -1,
        Optional = 0,
        Always = 1
    };

    /** Set this value to something other than max() to hard-cap the number of worker threads that may be created. */
    constexpr const size_t worker_thread_limit = std::numeric_limits<size_t>::max();

    /** The minimum number of elements in a requested matrix to trigger multi-threaded creation in optional mode. */
    constexpr const size_t minimum_matrix_element_count = 6400; // = 80 x 80 matrix, or larger.

    /** The minimum product of of elements in a requested matrix with log2 of the number of rules,
     * to trigger multi-threaded creation in optional mode. */
    constexpr const size_t minimum_rule_difficulty = 6400; // = 80 x 80 matrix with 1 rule, or harder.

    /** The minimum number of possible elements in an OSG to trigger multi-threaded creation in optional mode. */
    constexpr const size_t minimum_osg_element_count = 1000;

    /**
     * Query the maximum number of workers that may be created
     * (This is the smaller value of the number of physical cores recorded and the hard-coded maximum, if any.
     */
    [[nodiscard]] size_t get_max_worker_threads();

    /** Should the matrix creation be multithreaded ? */
    [[nodiscard]] bool should_multithread_matrix_creation(MultiThreadPolicy policy, size_t elements);

    /** Should the rule application be multithreaded ? */
    [[nodiscard]] bool should_multithread_rule_application(MultiThreadPolicy policy, size_t elements, size_t rules);

    [[nodiscard]] bool should_multithread_osg(MultiThreadPolicy policy, size_t potential_elements);

}