/**
 * verify_as_class.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <exception>
#include <map>
#include <optional>
#include <string>
#include <utility>
#include <variant>

#include "MatlabDataArray.hpp"
#include "utilities/reporting.h"
#include "error_codes.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex {

    namespace errors {
        struct bad_class_exception : public std::runtime_error {
            const std::string className;
            bad_class_exception(std::string className, const std::string& what) noexcept
                : std::runtime_error(what), className{std::move(className)} { }
        };
    }

    namespace classes {

        class MATLABClass {
        public:
            using ArrayOrReason = std::variant<matlab::data::Array, std::string>;
            using FieldTypeMap = std::map<std::string, matlab::data::ArrayType>;

        protected:
            matlab::engine::MATLABEngine &engine;
            matlab::data::Array raw_data;
            FieldTypeMap fields;

        public:
            const std::string className;
            const size_t num_elements = 0;

            MATLABClass(matlab::engine::MATLABEngine &engine,
                        std::string the_name,
                        FieldTypeMap &&fields,
                        matlab::data::Array rawInput);

            [[nodiscard]] matlab::data::Array property(const std::string &propertyName);

            template<typename data_type>
            [[nodiscard]] inline matlab::data::TypedArray<data_type> property_array(const std::string &propertyName) {
                return this->property(propertyName);
            }

            [[nodiscard]] inline matlab::data::StructArray property_struct(const std::string &propertyName) {
                return this->property(propertyName);
            }

            template<typename data_type>
            [[nodiscard]] inline data_type property_scalar(const std::string &propertyName) {
                auto array = this->property_array<data_type>(propertyName);
                if (array.isEmpty()) {
                    throw errors::bad_class_exception{this->className, std::string("Empty scalar property '")
                                                                       + propertyName + "'"};
                }
                return *(array.begin());
            }


            [[nodiscard]] matlab::data::Array property(size_t index, const std::string &propertyName);

            template<typename data_type>
            [[nodiscard]] inline matlab::data::TypedArray<data_type> property_array(size_t index,
                                                                                    const std::string &propertyName) {
                return this->property(index, propertyName);
            }

            [[nodiscard]] inline matlab::data::StructArray property_struct(size_t index,
                                                                           const std::string &propertyName) {
                return this->property(index, propertyName);
            }

            template<typename data_type>
            [[nodiscard]] inline data_type property_scalar(size_t index, const std::string &propertyName) {
                auto array = this->property_array<data_type>(index, propertyName);
                if (array.isEmpty()) {
                    throw errors::bad_class_exception{this->className, std::string("Empty scalar property '")
                                                                       + propertyName + "'"};
                }
                return *(array.begin());
            }

            [[nodiscard]] inline bool is_scalar() const noexcept {
                return (this->num_elements == 1);
            };

            [[nodiscard]] inline bool empty() const noexcept {
                return (this->num_elements == 0);
            };

            /**
             * Uses MATLAB's "isa" function to test if supplied object is a MATLAB class, or a handle to a class.
             * @param engine Reference to MATLAB engine.
             * @param input  The array to test.
             * @param className The name of the class to compare against.
             * @return
             */
            [[nodiscard]] static std::pair<bool, std::optional<std::string>>
            verify_as_class_handle(matlab::engine::MATLABEngine &engine,
                                   const matlab::data::Array &input, const std::string &className);

            /**
             * Test if class has property of supplied type, and retrieve property it if it does.
             * @param engine Reference to MATLAB engine.
             * @param input  The class object to test.
             * @param propertyName The name of the property to find.
             * @param type The type that the property should have.
             * @return The array, or a reason why it cannot be accessed.
             */
            [[nodiscard]] static ArrayOrReason verify_class_property(matlab::engine::MATLABEngine &engine,
                                                                     const matlab::data::Array &input,
                                                                     const std::string &propertyName,
                                                                     matlab::data::ArrayType type);

            /**
             * Test if class has property of supplied type at requested index, and retrieve property it if it does.
             * @param engine Reference to MATLAB engine.
             * @param input  The class object to test.
             * @param propertyName The name of the property to find.
             * @param type The type that the property should have.
             * @return The array, or a reason why it cannot be accessed.
             */
            [[nodiscard]] static ArrayOrReason verify_class_property(matlab::engine::MATLABEngine &engine,
                                                                     const matlab::data::Array &input,
                                                                     size_t index,
                                                                     const std::string &propertyName,
                                                                     matlab::data::ArrayType type);


        };

        [[nodiscard]] std::pair<bool, std::optional<std::string>>
        verify_struct(matlab::engine::MATLABEngine &engine,
                      const matlab::data::Array &input, const std::vector<std::string> &field_names);
    }
}

