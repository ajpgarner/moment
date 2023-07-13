/**
 * matrix_system_errors.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <stdexcept>
#include <string>

namespace Moment::errors {
    /**
     * Error issued when a component from the matrix system is requested, but does not exist.
     */
    class missing_component : public std::runtime_error {
    public:
        explicit missing_component(const std::string& what) : std::runtime_error{what} { }
    };

}