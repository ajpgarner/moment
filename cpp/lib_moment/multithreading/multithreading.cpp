/**
 * multithreading.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "multithreading.h"

#ifdef _WIN32
#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <Windows.h>
#else
#include <thread>
#endif

#include <algorithm>
#include <cmath>
#include <iostream>

namespace Moment::Multithreading {


    std::ostream& operator<<(std::ostream& os, MultiThreadPolicy policy) {
        os << to_string(policy);
        return os;
    }

    std::string to_string(MultiThreadPolicy policy) {
        switch (policy) {
            case MultiThreadPolicy::Never:
                return "Never";
            case MultiThreadPolicy::Optional:
                return "Optional";
            case MultiThreadPolicy::Always:
                return "Always";
            default: [[unlikely]]
                return std::string("Unknown (") + std::to_string(static_cast<int>(policy)) + ")";
        }
    }

    namespace {


#ifdef _WIN32
    size_t os_core_reporting() {
        // Use windows-reported value.
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return static_cast<size_t>(sysinfo.dwNumberOfProcessors);
    }
#else
    size_t os_core_reporting() {
        // Default implementation is to use value reported by std::thread.
        // Thanks to Intel, this value is almost always off by a factor of two on modern CPUs.
        auto hwc = std::thread::hardware_concurrency();
        if (hwc < 1) {
            hwc = 1;
        }
        return hwc;
    }
#endif
}

    size_t get_max_worker_threads() {
        static auto OS_cores = os_core_reporting();
        return std::min(OS_cores, worker_thread_limit);
    }

    bool should_multithread_matrix_creation(MultiThreadPolicy policy, size_t elements) noexcept {
        return should_multithread(policy, minimum_matrix_element_count, elements);
    }

    bool should_multithread_rule_application(MultiThreadPolicy policy, size_t elements, size_t rules) noexcept {
        const size_t difficulty = (rules <= 0) ? std::numeric_limits<size_t>::max()
                                               : elements * std::ceil(std::log2(static_cast<double>(rules)));
        return should_multithread(policy, minimum_rule_difficulty, difficulty);
    }

    bool should_multithread_group_rep_generation(MultiThreadPolicy policy,
                                                 const size_t raw_dim, const size_t group_elements) noexcept {
        const size_t difficulty = group_elements <= 1 ? std::numeric_limits<size_t>::max()
                                                      : (raw_dim * raw_dim * group_elements);
        return should_multithread(policy, minimum_group_rep_difficulty, difficulty);
    }

    bool should_multithread_osg(MultiThreadPolicy policy, size_t potential_elements) noexcept {
        return should_multithread(policy, minimum_osg_element_count, potential_elements);
    }

}
