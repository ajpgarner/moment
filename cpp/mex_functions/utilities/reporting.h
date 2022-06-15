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
    [[noreturn]] void throw_error(matlab::engine::MATLABEngine& engine,
                                  const std::string& err_code,
                                  const std::string& error);

    [[noreturn]] void throw_error(matlab::engine::MATLABEngine& engine,
                                  const std::string& err_code,
                                  const std::basic_string<char16_t>& error);

    void print_to_console(matlab::engine::MATLABEngine& engine,
                          const std::string& message);

    void print_to_console(matlab::engine::MATLABEngine& engine,
                          const std::basic_string<char16_t>& message);
}