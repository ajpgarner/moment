/**
 * read_as_int.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_as_scalar.h"
#include "io_parameters.h"
#include "visitor.h"
#include "utilities/utf_conversion.h"

#include <sstream>

namespace Moment::mex {

    namespace errors {

        void throw_unreadable_scalar(const std::string &paramName, const unreadable_scalar& urs) {
            std::stringstream ss;
            ss << paramName << " could not be read: " << urs.what();
            throw errors::BadInput{urs.errCode, ss.str()};
        }

        void throw_not_castable_to_scalar(const std::string &paramName) {
            std::stringstream ss;
            ss << paramName << " should be a scalar positive integer.";
            throw errors::BadInput{errors::bad_param, ss.str()};
        }

        void throw_under_min_scalar(const std::string &paramName, int64_t min_value) {
            std::stringstream ss;
            ss << paramName << " must have a value of at least "
               << min_value << ".";
            throw errors::BadInput{errors::bad_param, ss.str()};
        }

        void throw_over_max_scalar(const std::string &paramName, uint64_t max_value) {
            std::stringstream ss;
            ss << paramName << " must have a value of at least "
               << max_value << ".";
            throw errors::BadInput{errors::bad_param, ss.str()};
        }
    }


    namespace {
        template<std::integral output_type>
        class IntReaderVisitor {

        public:
            using return_type = output_type;

        public:
            explicit IntReaderVisitor() = default;

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @tparam datatype The data array type
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            template<std::convertible_to<return_type> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                if (data.isEmpty()) {
                    throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty array."};
                }
                if (data.getNumberOfElements() > 1) {
                    throw errors::unreadable_scalar{errors::not_a_scalar, "Not a scalar."};
                }

                // Extra check if possible that input value has negative sign, but we are casting to an unsigned value:
                if constexpr ((std::is_floating_point<datatype>::value
                                    ||  std::is_signed<datatype>::value)
                               && (std::is_unsigned<return_type>::value)) {
                    auto raw_read = *data.begin();
                    if (raw_read < 0) {
                        throw errors::unreadable_scalar{errors::negative_value, "Value unexpectedly negative."};
                    }
                    return static_cast<return_type>(raw_read);
                }

                // Otherwise, just directly cast:
                return static_cast<return_type>(*data.begin());
            }

            return_type string(const matlab::data::StringArray &data) {
                if (data.isEmpty()) {
                    throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty array."};
                }
                if (data.getNumberOfElements() > 1) {
                    throw errors::unreadable_scalar{errors::not_a_scalar, "Not a scalar."};
                }
                auto str = *data.begin();
                if (!str.has_value()) {
                    throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty string."};
                }

                try {
                    UTF16toUTF8Convertor convertor;
                    std::string utf8str = convertor(*str);

                    // Ensure string is not negative.
                    if constexpr (std::is_unsigned<return_type>::value) {
                        if (!utf8str.empty() && utf8str[0] == '-') {
                            throw errors::unreadable_scalar{errors::negative_value, "Value unexpectedly negative."};
                        }
                    }

                    std::stringstream ss{utf8str};
                    return_type output{};
                    ss >> output;
                    if (ss.fail()) {
                        std::stringstream errSS;
                        errSS << "Could not interpret string\"" << utf8str << "\" as integer.";
                        throw errors::unreadable_scalar{errors::could_not_convert, errSS.str()};
                    }
                    return output;
                } catch (errors::unreadable_scalar& urs) {
                    throw; // rethrow
                } catch (std::exception& e) {
                    throw errors::unreadable_scalar{errors::could_not_convert, "Could not convert string to integer."};
                }
            }

        };

        static_assert(concepts::VisitorHasRealDense<IntReaderVisitor<long>>);
        static_assert(concepts::VisitorHasString<IntReaderVisitor<long>>);

        template<std::floating_point output_type>
        class FloatReaderVisitor {

        public:
            using return_type = output_type;

        public:
            explicit FloatReaderVisitor() = default;

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @tparam datatype The data array type
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            template<std::convertible_to<return_type> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                if (data.isEmpty()) {
                    throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty array."};
                }
                if (data.getNumberOfElements() > 1) {
                    throw errors::unreadable_scalar{errors::not_a_scalar, "Not a scalar."};
                }

                // Just directly cast:
                return static_cast<return_type>(*data.begin());
            }

            return_type string(const matlab::data::StringArray &data) {
                if (data.isEmpty()) {
                    throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty array."};
                }
                if (data.getNumberOfElements() > 1) {
                    throw errors::unreadable_scalar{errors::not_a_scalar, "Not a scalar."};
                }
                auto str = *data.begin();
                if (!str.has_value()) {
                    throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty string."};
                }

                try {
                    UTF16toUTF8Convertor convertor;
                    std::string utf8str = convertor(*str);
                    std::stringstream ss{utf8str};
                    return_type output{};
                    ss >> output;
                    return output;
                } catch (std::exception& e) {
                    throw errors::unreadable_scalar{errors::could_not_convert, "Could not convert string to integer."};
                }
            }

        };

        static_assert(concepts::VisitorHasRealDense<FloatReaderVisitor<double>>);
        static_assert(concepts::VisitorHasString<FloatReaderVisitor<double>>);

        template<std::integral output_type>
        output_type do_read_as_scalar(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            return DispatchVisitor(engine, input, IntReaderVisitor<output_type>{});
        }

        template<std::floating_point output_type>
        output_type do_read_as_scalar(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            return DispatchVisitor(engine, input, FloatReaderVisitor<output_type>{});
        }
    }

    uint64_t read_as_scalar(matlab::engine::MATLABEngine &engine, const matlab::data::MATLABString& str) {
        if (!str.has_value()) {
            throw errors::unreadable_scalar{errors::empty_array, "Unexpected empty string."};
        }

        try {
            UTF16toUTF8Convertor convertor;
            std::string utf8str = convertor(*str);

            // Ensure string is not negative.
            if (!utf8str.empty() && utf8str[0] == '-') {
                throw errors::unreadable_scalar{errors::negative_value, "Value unexpectedly negative."};
            }

            // Read
            std::stringstream ss{utf8str};
            uint64_t output;
            ss >> output;

            return output;
        } catch (errors::unreadable_scalar& urs) {
            throw; // rethrow
        } catch (std::exception& e) {
            throw errors::unreadable_scalar{errors::could_not_convert, "Could not convert string to integer."};
        }
    }

    int16_t read_as_int16(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<int16_t>(engine, input);
    }

    int32_t read_as_int32(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<int32_t>(engine, input);
    }

    int64_t read_as_int64(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<int64_t>(engine, input);
    }

    uint16_t read_as_uint16(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<uint16_t>(engine, input);
    }

    uint32_t read_as_uint32(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<uint32_t>(engine, input);
    }

    uint64_t read_as_uint64(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<uint64_t>(engine, input);
    }

    float read_as_float(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<float>(engine, input);
    }

    double read_as_double(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return do_read_as_scalar<double>(engine, input);
    }

    bool castable_to_scalar_int(const matlab::data::Array &input) {
        if (input.isEmpty()) {
            return false;
        }

        if (input.getNumberOfElements() != 1) {
            return false;
        }

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

    bool castable_to_scalar_float(const matlab::data::Array &input) {
        // Same criteria as int:
        return castable_to_scalar_int(input);
    }
}
