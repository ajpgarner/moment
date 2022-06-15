/**
 * parse_to_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/context.h"
#include <memory>
#include <string>
#include <optional>

#include "MatlabDataArray.hpp"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex {
    /**
     * Check if supplied array object matches a valid specification of (matlab) 'Setting' class.
     * @param engine Handle to MATLAB engine.
     * @param raw_data The raw object to verify.
     * @return Pair: first: true if object is valid Setting; second: why verification failed, set when first is false.
     */
    [[nodiscard]] std::pair<bool, std::optional<std::string>>
    verify_as_setting(matlab::engine::MATLABEngine& engine, const matlab::data::Array& raw_data);

    /**
     * Assumes the supplied raw_data is a 'Setting' matlab class, and parse it accordingly into a Context C++ class.
     * @param engine Handle to MATLAB engine.
     * @param raw_data The Setting class object. Sanitize this input with verify_as_setting first!
     * @return Owning pointer to newly constructed NPATK::Context class.
     */
    [[nodiscard]] std::unique_ptr<Context> parse_to_context(matlab::engine::MATLABEngine& engine,
                                                            matlab::data::Array& raw_data);

}