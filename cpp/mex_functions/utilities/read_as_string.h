/**
 * read_as_string.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>
#include <optional>

#include "mex.hpp"
#include "MatlabDataArray.hpp"

namespace Moment::mex {

    std::optional<std::basic_string<char16_t>> read_as_utf16(matlab::data::Array input);

}

