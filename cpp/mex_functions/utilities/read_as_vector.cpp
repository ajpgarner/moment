/**
 * read_as_vector.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_as_vector.h"
#include "read_as_scalar.h"

#include "io_parameters.h"

#include "visitor.h"

#include <utility>

namespace Moment::mex {

    namespace errors {
        void throw_not_castable_to_vector(const std::string& paramName) {
            std::stringstream ss;
            ss << paramName << " should be a vector of positive integers.";
            throw errors::BadInput{errors::bad_param, ss.str()};
        }

        void throw_unreadable_vector(const std::string &paramName, const unreadable_vector& urv) {
            std::stringstream ss;
            ss << paramName << " could not be read: " << urv.what();
            throw errors::BadInput{urv.errCode, ss.str()};
        }

        void throw_under_min_vector(const std::string &paramName, uint64_t min_value) {
            std::stringstream ss;
            ss << "All elements of " << paramName << " must have a value of at least "
               << min_value << ".";
            throw errors::BadInput{errors::bad_param, ss.str()};
        }
    }

    namespace {


        template<std::integral value_type>
        class IntVectorReaderVisitor {
        public:
            using return_type = std::vector<value_type>;

        public:
            explicit IntVectorReaderVisitor() = default;

            template<std::convertible_to<value_type> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                if (data.isEmpty()) {
                    return return_type{};
                }

                return_type output;
                output.reserve(data.getNumberOfElements());

                for (const auto& val : data) {
                    // First check positivity (if returning unsigned integer)
                    if constexpr ((std::is_floating_point<datatype>::value
                                   ||  std::is_signed<datatype>::value)
                                  && (std::is_unsigned<value_type>::value)) {
                        if (val < 0) {
                            throw errors::unreadable_vector{errors::negative_value, "Value unexpectedly negative."};
                        }
                    }
                    output.emplace_back(static_cast<value_type>(val));
                }

                return output;
            }

            return_type string(const matlab::data::StringArray &data) {

                if (data.isEmpty()) {
                    return return_type{};
                }

                return_type output;
                output.reserve(data.getNumberOfElements());

                for (const auto& str : data) {

                    if (!str.has_value()) {
                        throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty string."};
                    }

                    try {
                        std::string utf8str = matlab::engine::convertUTF16StringToUTF8String(*str);

                        // Ensure string is not negative.
                        if constexpr (std::is_unsigned<return_type>::value) {
                            if (!utf8str.empty() && utf8str[0] == '-') {
                                throw errors::unreadable_vector{errors::negative_value, "Value unexpectedly negative."};
                            }
                        }

                        std::stringstream ss{utf8str};
                        value_type read_buf;
                        ss >> read_buf;
                        if (ss.fail()) {
                            std::stringstream errSS;
                            errSS << "Could not interpret string\"" << utf8str << "\" as integer.";
                            throw errors::unreadable_vector{errors::could_not_convert, errSS.str()};
                        }
                        output.emplace_back(read_buf);
                    } catch (errors::unreadable_vector &urs) {
                        throw; // rethrow
                    } catch (std::exception &e) {
                        throw errors::unreadable_vector{errors::could_not_convert,
                                                        "Could not convert string to integer."};
                    }
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<IntVectorReaderVisitor<int64_t>>);
        static_assert(concepts::VisitorHasString<IntVectorReaderVisitor<int64_t>>);
        static_assert(concepts::VisitorHasRealDense<IntVectorReaderVisitor<uint64_t>>);
        static_assert(concepts::VisitorHasString<IntVectorReaderVisitor<uint64_t>>);


        template<std::floating_point value_type>
        class FloatVectorReaderVisitor {
        public:
            using return_type = std::vector<value_type>;

        public:
            explicit FloatVectorReaderVisitor() = default;

            template<std::convertible_to<value_type> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                if (data.isEmpty()) {
                    return return_type{};
                }

                return_type output;
                output.reserve(data.getNumberOfElements());

                for (const auto& val : data) {
                    output.emplace_back(static_cast<value_type>(val));
                }

                return output;
            }

            return_type string(const matlab::data::StringArray &data) {

                if (data.isEmpty()) {
                    return return_type{};
                }

                return_type output;
                output.reserve(data.getNumberOfElements());

                for (const auto& str : data) {

                    if (!str.has_value()) {
                        throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty string."};
                    }

                    try {
                        std::string utf8str = matlab::engine::convertUTF16StringToUTF8String(*str);

                        std::stringstream ss{utf8str};
                        value_type read_buf;
                        ss >> read_buf;
                        if (ss.fail()) {
                            std::stringstream errSS;
                            errSS << "Could not interpret string\"" << utf8str << "\" as floating point.";
                            throw errors::unreadable_vector{errors::could_not_convert, errSS.str()};
                        }
                        output.emplace_back(read_buf);
                    } catch (errors::unreadable_vector &urs) {
                        throw; // rethrow
                    } catch (std::exception &e) {
                        throw errors::unreadable_vector{errors::could_not_convert,
                                                        "Could not convert string to floating point."};
                    }
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<FloatVectorReaderVisitor<float>>);
        static_assert(concepts::VisitorHasString<FloatVectorReaderVisitor<float>>);
        static_assert(concepts::VisitorHasRealDense<FloatVectorReaderVisitor<double>>);
        static_assert(concepts::VisitorHasString<FloatVectorReaderVisitor<double>>);

        template<std::integral value_type>
        std::vector<value_type> do_read_as_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            return DispatchVisitor(engine, input, IntVectorReaderVisitor<value_type>{});
        }

        template<std::floating_point value_type>
        std::vector<value_type> do_read_as_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            return DispatchVisitor(engine, input, FloatVectorReaderVisitor<value_type>{});
        }

    }

    std::vector<int16_t>
    read_as_int16_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<int16_t>(engine, input);
    }

    std::vector<uint16_t>
    read_as_uint16_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<uint16_t>(engine, input);
    }

    std::vector<int32_t>
    read_as_int32_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<int32_t>(engine, input);
    }

    std::vector<uint32_t>
    read_as_uint32_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<uint32_t>(engine, input);
    }

    std::vector<int64_t>
    read_as_int64_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<int64_t>(engine, input);
    }

    std::vector<uint64_t>
    read_as_uint64_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<uint64_t>(engine, input);
    }

    std::vector<float>
    read_as_float_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<float>(engine, input);
    }

    std::vector<double>
    read_as_double_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return do_read_as_vector<double>(engine, input);
    }

    bool castable_to_vector_int(const matlab::data::Array &input) {
        switch(input.getType()) {
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
            case matlab::data::ArrayType::MATLAB_STRING: // with conversion
                return true;
            default:
                return false;
        }
    }

}