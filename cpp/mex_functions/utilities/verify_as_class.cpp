/**
 * verify_as_class.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "verify_as_class.h"

#include "cppmex/mexMatlabEngine.hpp"
#include "cppmex/mexException.hpp"

#include <sstream>

namespace NPATK::mex {


    [[nodiscard]] std::pair<bool, std::optional<std::string>>
    verify_as_class_handle(matlab::engine::MATLABEngine& engine,
                           const matlab::data::Array& raw_data, const std::string& className) {

        // Only check basic type if not empty.
        if (!raw_data.isEmpty()) {
            switch (raw_data.getType()) {
                case matlab::data::ArrayType::HANDLE_OBJECT_REF:
                case matlab::data::ArrayType::OBJECT:
                    break;
                default: {
                    std::stringstream errSS;
                    errSS << "Not a valid " << className << " object.";
                    return {false, errSS.str()};
                }
            }
        }

        // Use isa function to test for correct class
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args { raw_data, factory.createCharArray(className) };
        matlab::data::TypedArray<bool> result = engine.feval(u"isa", args);
        if (result[0] != true) {
            std::stringstream errSS;
            errSS << "Not a valid handle to instance of " << className << " object.";
            return {false, errSS.str()};
        }
        return {true, {}};
    }


    std::pair<bool, std::optional<std::string>>
    verify_struct(matlab::engine::MATLABEngine &engine, const matlab::data::Array &raw_input,
                  const std::vector<std::string> &field_names) {
        if (raw_input.getType() != matlab::data::ArrayType::STRUCT) {
            return {false, "Not a structure."};
        }
        const auto struct_input = static_cast<const matlab::data::StructArray>(raw_input);
        if (struct_input.getNumberOfFields() != field_names.size()) {
            return {false, "Mismatched number of fields."};
        }

        size_t field_index = 0;
        for (const auto& fieldName : struct_input.getFieldNames()) {
            if (field_names[field_index] != fieldName) {
                return {false, std::string("Expected field '") + field_names[field_index]
                                            + "', but instead found field '" + std::string(fieldName) + "'"};
            }
            ++field_index;
        }

        return {true, {}};
    }

    std::optional<matlab::data::Array>
    try_get_property(matlab::engine::MATLABEngine &engine,
                     const matlab::data::Array &input,
                     const std::string &propertyName) {
        try {
            return engine.getProperty(input, propertyName);
        } catch (const matlab::engine::MATLABException& ex) {
            // Only catch invalid property
            if (!(ex.getMessageID() == "MATLAB:class:InvalidProperty")) {
                throw; // rethrow
            }
            // Otherwise, return not found
            return {};
        }
    }

    std::optional<matlab::data::Array>
    try_get_property(matlab::engine::MATLABEngine &engine,
                     const matlab::data::Array &input,
                     size_t index,
                     const std::string &propertyName) {
        try {
            return engine.getProperty(input, index, propertyName);
        } catch (const matlab::engine::MATLABException& ex) {
            // Only catch invalid property
            if (!(ex.getMessageID() == "MATLAB:class:InvalidProperty")) {
                throw; // rethrow
            }
            // Otherwise, return not found
            return {};
        }
    }

}