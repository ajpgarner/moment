/**
 * reflection.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>
#include "MatlabDataArray.hpp"

namespace NPATK::mex {
    /**
     * Get a string describing the array type.
     */
    [[nodiscard]] std::string type_as_string(const matlab::data::Array& array);

    /**
     * Get a string describing the array's type and dimensions.
     */
    [[nodiscard]] std::string summary_string(const matlab::data::Array& array);
}