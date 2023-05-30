/**
 * multithreading.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <limits>

#include "integer_types.h"

namespace Moment {

    /** Set this value to something other than max() to hard-cap the number of worker threads that may be created. */
    constexpr const size_t worker_thread_limit = std::numeric_limits<size_t>::max();

    /** Query the maximum number of workers that may be created
     * (This is the smaller value of the number of physical cores recorded and the hard-coded maximum, if any.
     */
    [[nodiscard]] size_t get_max_worker_threads();
}