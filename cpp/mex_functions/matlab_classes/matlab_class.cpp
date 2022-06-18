/**
 * verify_as_class.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "matlab_class.h"
#include "utilities/reflection.h"

#include "cppmex/mexMatlabEngine.hpp"
#include "cppmex/mexException.hpp"
#include "cppmex/detail/mexExceptionImpl.hpp"

#include <sstream>

namespace NPATK::mex::classes {

    MATLABClass::MATLABClass(matlab::engine::MATLABEngine &engine,
                             std::string the_name,
                             FieldTypeMap&& fieldSpec,
                             matlab::data::Array rawInput)
                 : engine{engine}, raw_data{std::move(rawInput)}, fields{std::move(fieldSpec)},
                  className{std::move(the_name)}, num_elements{raw_data.getNumberOfElements()} {

        // Check with MATLAB that class is valid.
        auto [validHandle, whyNotValidHandle] = verify_as_class_handle(engine, raw_data, className);
        if (!validHandle) {
            throw errors::bad_class_exception{className, whyNotValidHandle.value()};
        }

        // Check fields exist
        for (const auto& [fieldName, fieldType] : this->fields) {
            auto fieldOrError = verify_class_property(engine, raw_data, fieldName, fieldType);
            if (fieldOrError.index() == 1) {
                throw errors::bad_class_exception{className, std::get<1>(fieldOrError)};
            }
        }
    }


    [[nodiscard]] std::pair<bool, std::optional<std::string>>
    MATLABClass::verify_as_class_handle(matlab::engine::MATLABEngine& engine,
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

    namespace {
        template<bool hasIndex>
        inline MATLABClass::ArrayOrReason do_verify_class_property(matlab::engine::MATLABEngine& engine,
            const matlab::data::Array& input,
            const std::string& propertyName,
            matlab::data::ArrayType expectedType,
            size_t index = 0) {

            try {
                auto property_array = hasIndex ? engine.getProperty(input, index, propertyName)
                                               : engine.getProperty(input, propertyName);

                if (property_array.getType() != expectedType) {
                    // Return bad type
                    std::stringstream errSS;
                    errSS << "Property '" << propertyName << "' was " << type_as_string(property_array)
                          << ", but " << to_string(expectedType) << " was expected.";
                    return {errSS.str()};
                }
                return std::move(property_array);
            } catch (const matlab::engine::MATLABException& ex) {
                // Only catch invalid property
                if (!(ex.getMessageID() == "MATLAB:class:InvalidProperty")) {
                    throw; // rethrow
                }
                // Otherwise, return not found
                std::stringstream errSS;
                errSS << "Property '" << propertyName << "' not found.";
                return {errSS.str()};
            }
        }
    }

    MATLABClass::ArrayOrReason MATLABClass::verify_class_property(matlab::engine::MATLABEngine& engine,
                                                                  const matlab::data::Array& input,
                                                                  size_t index,
                                                                  const std::string& propertyName,
                                                                  matlab::data::ArrayType expectedType) {
        return do_verify_class_property<true>(engine, input, propertyName, expectedType, index);
    }

    MATLABClass::ArrayOrReason MATLABClass::verify_class_property(matlab::engine::MATLABEngine& engine,
                                                                  const matlab::data::Array& input,
                                                                  const std::string& propertyName,
                                                                  matlab::data::ArrayType expectedType) {
        return do_verify_class_property<false>(engine, input, propertyName, expectedType);
    }


    matlab::data::Array MATLABClass::property(const std::string &propertyName) {
        auto type_iter = this->fields.find(propertyName);
        if (type_iter == this->fields.end()) {
            std::stringstream ss;
            ss << "Unlisted property '" << propertyName << "'.";
            throw errors::bad_class_exception{this->className, ss.str()};
        }

        auto propertyOrNot = verify_class_property(this->engine, this->raw_data, propertyName, type_iter->second);
        if (1 == propertyOrNot.index()) {
            throw errors::bad_class_exception{this->className, std::get<1>(propertyOrNot)};
        }
        return get<0>(propertyOrNot);
    }

    matlab::data::Array MATLABClass::property(size_t index, const std::string &propertyName) {
        auto type_iter = this->fields.find(propertyName);
        if (type_iter == this->fields.end()) {
            std::stringstream ss;
            ss << "Unlisted property '" << propertyName << "'.";
            throw errors::bad_class_exception{this->className, ss.str()};
        }

        auto propertyOrNot = verify_class_property(this->engine, this->raw_data,
                                                   index, propertyName,
                                                   type_iter->second);
        if (1 == propertyOrNot.index()) {
            throw errors::bad_class_exception{this->className, std::get<1>(propertyOrNot)};
        }
        return get<0>(propertyOrNot);
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
}