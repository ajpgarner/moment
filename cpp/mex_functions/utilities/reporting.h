/**
 * reporting.h
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <string>

#include "mex.hpp"
#include "MatlabDataArray.hpp"

namespace Moment::mex {
    void print_warning(matlab::engine::MATLABEngine& engine, const std::string& warning);

    void print_to_console(matlab::engine::MATLABEngine& engine, const std::string& message);

    void print_to_console(matlab::engine::MATLABEngine& engine, const std::basic_string<char16_t>& message);
}