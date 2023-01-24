/**
 * read_choice.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray.hpp"

#include <concepts>
#include <stdexcept>
#include <vector>

namespace Moment::mex {

    namespace errors {
        class invalid_choice : public std::range_error {
        public:
            invalid_choice(const std::string& what) : std::range_error{what} { }
        };
    }

    /**
     * Gets whether input represents a string that matches one of the choices given.
     * @param param_name The name of the choice parameter, for error message purposes
     * @param choices A list of valid, case-insensitive, choices
     * @param input The matlab data object
     * @return The matched index of the choice from choices
     */
    size_t read_choice(const std::string& param_name,
                       std::vector<std::string> choices,
                       matlab::data::Array input);

}
