/**
 * verify_as_class.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <optional>
#include <string>
#include <utility>

#include "MatlabDataArray.hpp"
#include "reporting.h"
#include "error_codes.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex {

    [[nodiscard]] std::pair<bool, std::optional<std::string>>
    verify_as_class_handle(matlab::engine::MATLABEngine& engine,
                           const matlab::data::Array& input, const std::string& className);

    [[nodiscard]] std::pair<bool, std::optional<std::string>>
    verify_struct(matlab::engine::MATLABEngine& engine,
                  const matlab::data::Array& input, const std::vector<std::string>& field_names);


    [[nodiscard]] std::optional<matlab::data::Array> try_get_property(matlab::engine::MATLABEngine& engine,
                                                                      const matlab::data::Array& input,
                                                                      const std::string& propertyName);

    [[nodiscard]] std::optional<matlab::data::Array> try_get_property(matlab::engine::MATLABEngine& engine,
                                                                      const matlab::data::Array& input,
                                                                      size_t index,
                                                                      const std::string& propertyName);


    template<typename type>
    inline matlab::data::TypedArray<type> get_property(matlab::engine::MATLABEngine& engine,
                                                const matlab::data::Array& input,
                                                const std::string& propertyName) {
        auto maybeProp = try_get_property(engine, input, propertyName);
        if (!maybeProp.has_value()) {
            throw_error(engine, errors::internal_error, std::string("Property '") + propertyName + "' not found");
        }
        return *maybeProp;
    }


    inline matlab::data::StructArray get_property_struct(matlab::engine::MATLABEngine& engine,
                                                         const matlab::data::Array& input,
                                                         const std::string& propertyName) {
        auto maybeProp = try_get_property(engine, input, propertyName);
        if (!maybeProp.has_value()) {
            throw_error(engine, errors::internal_error, std::string("Property '") + propertyName + "' not found");
        }
        return *maybeProp;
    }

    template<typename type>
    inline matlab::data::TypedArray<type> get_property(matlab::engine::MATLABEngine& engine,
                                                const matlab::data::Array& input,
                                                size_t index,
                                                const std::string& propertyName) {
        auto maybeProp = try_get_property(engine, input, index, propertyName);
        if (!maybeProp.has_value()) {
            throw_error(engine, errors::internal_error, std::string("Index ") + std::to_string(index)
                                                            + ": Property '" + propertyName + "' not found");
        }
        return *maybeProp;
    }

    inline matlab::data::StructArray get_property_struct(matlab::engine::MATLABEngine& engine,
                                                const matlab::data::Array& input,
                                                size_t index,
                                                const std::string& propertyName) {
        auto maybeProp = try_get_property(engine, input, index, propertyName);
        if (!maybeProp.has_value()) {
            throw_error(engine, errors::internal_error, std::string("Index ") + std::to_string(index)
                                                            + ": Property '" + propertyName + "' not found");
        }
        return *maybeProp;
    }

}

