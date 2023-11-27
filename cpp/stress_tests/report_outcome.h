/**
 * report_outcome.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <chrono>
#include <iostream>
#include <stdexcept>


namespace Moment::StressTests {

    void report_failure(const auto& start_time, const std::exception& e) {
        const auto failure_time = std::chrono::high_resolution_clock::now();
        std::chrono::duration<double> the_duration = failure_time - start_time;

        std::cout << "...failed after ";
        if (the_duration >= std::chrono::duration<double>(1.0)) {
            std::cout << the_duration;
        } else {
            const std::chrono::duration<double, std::milli> ms_duration = the_duration;
            std::cout << ms_duration;
        }
        std::cout << ": " << e.what() << std::endl;
    }

    void report_success(const auto& start_time) {
        const auto success_time = std::chrono::high_resolution_clock::now();
        const std::chrono::duration<double> the_duration = success_time - start_time;
        std::cout << "... done in ";
        if (the_duration >= std::chrono::duration<double>(1.0)) {
            std::cout << the_duration;
        } else {
            const std::chrono::duration<double, std::milli> ms_duration = the_duration;
            std::cout << ms_duration;
        }
        std::cout << "." << std::endl;
    }
}