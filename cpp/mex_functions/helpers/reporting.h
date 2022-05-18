/**
 * reporting.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>

#include "mex.hpp"
#include "MatlabDataArray.hpp"

namespace NPATK::mex {
    void throw_error(matlab::engine::MATLABEngine& engine, const std::string& error);
    void throw_error(matlab::engine::MATLABEngine& engine, const std::basic_string<char16_t>& error);
    void print_to_console(matlab::engine::MATLABEngine& engine, const std::string& message);
}