/**
 * symmetrized_errors.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <stdexcept>
#include <string>

namespace Moment::errors {
    /**
     * Thrown if there is a problem with a map.
     */
    class bad_map : public std::runtime_error {
    public:
        explicit bad_map(const std::string& what) noexcept : std::runtime_error{what} { }
    };

    /**
     * Thrown if there is a problem with the solution of a map core.
     */
    class invalid_solution : public bad_map {
    public:
        explicit invalid_solution(const std::string& what) noexcept : bad_map(what) { }
    };
}
