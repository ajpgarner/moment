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
    void debug_message(matlab::engine::MATLABEngine& engine, const std::string& message);
}