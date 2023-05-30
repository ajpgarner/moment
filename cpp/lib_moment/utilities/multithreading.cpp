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
#include <windows.h>
#else
#ifdef UNIX
#include "sys/sysinfo.h"
#else
#include <thread>
#endif
#endif

#include <algorithm>


namespace Moment::Multithreading {

    namespace {


#ifdef _WIN32
    size_t os_core_reporting() {
        // Use windows-reported value.
        SYSTEM_INFO sysinfo;
        GetSystemInfo(&sysinfo);
        return static_cast<size_t>(sysinfo.dwNumberOfProcessors);
    }
#else
#ifdef UNIX
    size_t os_core_reporting() {
        // Use GNU/linux reported value
        auto hwc = get_nprocs();
        if (hwc < 1) {
            hwc = 1;
        }
        return hwc;
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
#endif
}

    size_t get_max_worker_threads() {
        static auto OS_cores = os_core_reporting();
        return std::min(OS_cores, worker_thread_limit);
    }

    bool should_multithread_matrix_creation(MultiThreadPolicy policy, size_t elements) {
        switch (policy) {
            case MultiThreadPolicy::Never:
                return false;
            case MultiThreadPolicy::Always:
                return true;
            case MultiThreadPolicy::Optional:
            default:
                return elements >= minimum_matrix_element_count;
        }
    }

    bool should_multithread_osg(MultiThreadPolicy policy, size_t potential_elements) {
        switch (policy) {
            case MultiThreadPolicy::Never:
                return false;
            case MultiThreadPolicy::Always:
                return true;
            case MultiThreadPolicy::Optional:
            default:
                return potential_elements >= minimum_osg_element_count;
        }
    }

}