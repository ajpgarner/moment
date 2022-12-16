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

namespace Moment::mex {

    namespace errors {
        struct bad_class_exception : public std::runtime_error {
            const std::string className;
            bad_class_exception(std::string className, const std::string& what) noexcept
                : std::runtime_error(what), className{std::move(className)} { }
        };
    }

    namespace classes {

        class DataSource {
        protected:
            matlab::engine::MATLABEngine& engine;

        public:
            explicit DataSource(matlab::engine::MATLABEngine &engine) : engine{engine} { }

            virtual ~DataSource() = default;

            [[nodiscard]] virtual matlab::data::Array get_property(const std::string &propertyName) = 0;

            [[nodiscard]] virtual bool is_a(const std::string& className) const = 0;

            [[nodiscard]] virtual bool has_class_type() const = 0;
        };

        class OwningArraySource : public DataSource {
            matlab::data::Array raw_data;

        public:
            explicit OwningArraySource(matlab::engine::MATLABEngine &engine, matlab::data::Array data)
                : DataSource(engine), raw_data(std::move(data)) { }

            [[nodiscard]] matlab::data::Array get_property(const std::string &propertyName) override;

            [[nodiscard]] bool is_a(const std::string& className) const override;

            [[nodiscard]] bool has_class_type() const override;
        };


        class IndexedRefSource : public DataSource {
            matlab::data::Array& raw_data_ref;
            const size_t data_index;

        public:
            IndexedRefSource(matlab::engine::MATLABEngine& engine, matlab::data::Array& array, size_t index)
                : DataSource{engine}, raw_data_ref{array}, data_index{index} {

            }

            [[nodiscard]] matlab::data::Array get_property(const std::string &propertyName) override;

            [[nodiscard]] bool is_a(const std::string& className) const override;

            [[nodiscard]] bool has_class_type() const override;
        };


        class MATLABClass {
        public:
            using ArrayOrReason = std::variant<matlab::data::Array, std::string>;
            using FieldTypeMap = std::map<std::string, matlab::data::ArrayType>;

        protected:
            matlab::engine::MATLABEngine &engine;

            std::unique_ptr<DataSource> data_source;

            FieldTypeMap fields;

        public:
            const std::string className;

        private:
            MATLABClass(matlab::engine::MATLABEngine &engine,
                        std::string the_name,
                        FieldTypeMap &&fields,
                        std::unique_ptr<DataSource> src);

        public:
            MATLABClass(matlab::engine::MATLABEngine &engine,
                        std::string the_name,
                        FieldTypeMap &&fields,
                        matlab::data::Array rawInput);

            MATLABClass(matlab::engine::MATLABEngine &engine,
                        std::string the_name,
                        FieldTypeMap &&fields,
                        matlab::data::Array& refInput,
                        size_t dataIndex);

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

            /**
             * Uses MATLAB's "isa" function to test if supplied object is a MATLAB class, or a handle to a class.
             * @param engine Reference to MATLAB engine.
             * @param input  The array to test.
             * @param className The name of the class to compare against.
             * @return
             */
            [[nodiscard]] static std::pair<bool, std::optional<std::string>>
            verify_as_class_handle(matlab::engine::MATLABEngine &engine,
                                   DataSource& input, const std::string &className);

            /**
             * Test if class has property of supplied type, and retrieve property it if it does.
             * @param engine Reference to MATLAB engine.
             * @param input  The class object to test.
             * @param propertyName The name of the property to find.
             * @param type The type that the property should have.
             * @return The array, or a reason why it cannot be accessed.
             */
            [[nodiscard]] static ArrayOrReason verify_class_property(matlab::engine::MATLABEngine &engine,
                                                                     DataSource& input,
                                                                     const std::string &propertyName,
                                                                     matlab::data::ArrayType type);
        };

    }
}

