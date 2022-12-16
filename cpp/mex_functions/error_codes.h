/**
 * error_codes.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>

namespace Moment::mex::errors {
    /** Prefix to error codes */
    constexpr char prefix[] = "mtk:";

    /** Applies prefix to code */
    constexpr std::string applyPrefix(const std::string& errCode) {
        return std::string(prefix) + errCode;
    }

    /** Error code: thrown when logical assertion fails, and it's not the users (direct) fault. */
    constexpr char internal_error[] = "internal_error";

    /** Error code: thrown when function is not recognised */
    constexpr char bad_function[] = "bad_function";

    /** Error code: thrown when known parameter encountered, but input following was bad. */
    constexpr char bad_param[] = "bad_param";

    /** Error code: thrown when a named paramter should be present, but is not */
    constexpr char missing_param[] = "missing_param";

    /** Error code: thrown when two or more mutually exclusive flags/parameters are provided. */
    constexpr char mutex_param[] = "mutex_param";

    /** Error code: thrown when inputs are missing */
    constexpr char too_few_inputs[] = "too_few_inputs";

    /** Error code: thrown when there are too many inputs */
    constexpr char too_many_inputs[] = "too_many_inputs";

    /** Error code: thrown when outputs are missing */
    constexpr char too_few_outputs[] = "too_few_outputs";

    /** Error code: thrown when there are too many outputs */
    constexpr char too_many_outputs[] = "too_many_outputs";

    /** Error code: thrown when a dynamic cast fails */
    constexpr char bad_cast[] = "bad_cast";

}
