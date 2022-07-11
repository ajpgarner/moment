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



    matlab::data::Array OwningArraySource::get_property(const std::string &propertyName) {
        return this->engine.getProperty(this->raw_data, propertyName);
    }

    bool OwningArraySource::is_a(const std::string& className) const {
        // Use isa function to test for correct class
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args { this->raw_data, factory.createCharArray(className) };
        matlab::data::TypedArray<bool> result = this->engine.feval(u"isa", args);
        return !result.isEmpty() && result[0];
    }

    bool OwningArraySource::has_class_type() const {
        // Cannot conclude anything if empty
        if (raw_data.isEmpty()) {
            return true;
        }

        switch (raw_data.getType()) {
            case matlab::data::ArrayType::HANDLE_OBJECT_REF:
            case matlab::data::ArrayType::OBJECT:
                return true;
            default:
                return false;
        }
    }

    matlab::data::Array IndexedRefSource::get_property(const std::string &propertyName) {
        return this->engine.getProperty(this->raw_data_ref, this->data_index, propertyName);
    }

    bool IndexedRefSource::is_a(const std::string& className) const {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args { this->raw_data_ref, factory.createCharArray(className) };
        matlab::data::TypedArray<bool> result = engine.feval(u"isa", args);
        return !result.isEmpty() && result[0];
    }

    bool IndexedRefSource::has_class_type() const {
        // Cannot conclude anything if empty
        if (raw_data_ref.isEmpty()) {
            return true;
        }

        switch (raw_data_ref.getType()) {
            case matlab::data::ArrayType::HANDLE_OBJECT_REF:
            case matlab::data::ArrayType::OBJECT:
                return true;
            default:
                return false;
        }
    }


    MATLABClass::MATLABClass(matlab::engine::MATLABEngine &engine, std::string the_name,
                             MATLABClass::FieldTypeMap &&fieldSpec, std::unique_ptr<DataSource> src) :
            engine{engine}, fields{std::move(fieldSpec)}, className{std::move(the_name)}, data_source{std::move(src)} {

        // Check with MATLAB that class is valid.
        auto [validHandle, whyNotValidHandle] = verify_as_class_handle(engine, *data_source, className);
        if (!validHandle) {
            throw errors::bad_class_exception{className, whyNotValidHandle.value()};
        }

        // Check fields exist
        for (const auto& [fieldName, fieldType] : this->fields) {
            auto fieldOrError = verify_class_property(engine, *data_source, fieldName, fieldType);
            if (fieldOrError.index() == 1) {
                throw errors::bad_class_exception{className, std::get<1>(fieldOrError)};
            }
        }
    }

    MATLABClass::MATLABClass(matlab::engine::MATLABEngine &engine,
                             std::string the_name,
                             FieldTypeMap&& fieldSpec,
                             matlab::data::Array rawInput)
             : MATLABClass(engine, std::move(the_name), std::move(fieldSpec),
                           std::make_unique<OwningArraySource>(engine, std::move(rawInput))) {

    }

    MATLABClass::MATLABClass(matlab::engine::MATLABEngine &engine,
                             std::string the_name,
                             FieldTypeMap&& fieldSpec,
                             matlab::data::Array& refInput,
                             size_t dataIndex)
             : MATLABClass(engine, std::move(the_name), std::move(fieldSpec),
                           std::make_unique<IndexedRefSource>(engine, refInput, dataIndex)) {
    }

    [[nodiscard]] std::pair<bool, std::optional<std::string>>
    MATLABClass::verify_as_class_handle(matlab::engine::MATLABEngine& engine,
                                        DataSource& dataSrc, const std::string& className) {

        // Only check basic type if not empty.
        if (!dataSrc.has_class_type()) {
            std::stringstream errSS;
            errSS << "Not a valid " << className << " object.";
            return {false, errSS.str()};
        }

        // Use isa function to test for correct class
        if (!dataSrc.is_a(className)) {
            std::stringstream errSS;
            errSS << "Not a valid handle to instance of " << className << " object.";
            return {false, errSS.str()};
        }
        return {true, {}};
    }

    MATLABClass::ArrayOrReason MATLABClass::verify_class_property(matlab::engine::MATLABEngine& engine,
                                                                  DataSource& input,
                                                                  const std::string& propertyName,
                                                                  matlab::data::ArrayType expectedType) {
        try {
            auto property_array = input.get_property(propertyName);
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


    matlab::data::Array MATLABClass::property(const std::string &propertyName) {
        auto type_iter = this->fields.find(propertyName);
        if (type_iter == this->fields.end()) {
            std::stringstream ss;
            ss << "Unlisted property '" << propertyName << "'.";
            throw errors::bad_class_exception{this->className, ss.str()};
        }

        auto propertyOrNot = verify_class_property(this->engine, *this->data_source, propertyName, type_iter->second);
        if (1 == propertyOrNot.index()) {
            throw errors::bad_class_exception{this->className, std::get<1>(propertyOrNot)};
        }
        return get<0>(propertyOrNot);
    }

}