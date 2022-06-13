/**
 * read_as_int.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "read_as_scalar.h"
#include "visitor.h"
#include "reporting.h"

#include <sstream>

namespace NPATK::mex {


    namespace {
        template<typename output_type>
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
                    std::string utf8str = matlab::engine::convertUTF16StringToUTF8String(*str);

                    // Ensure string is not negative.
                    if constexpr (std::is_unsigned<return_type>::value) {
                        if (!utf8str.empty() && utf8str[0] == '-') {
                            throw errors::unreadable_scalar{errors::negative_value, "Value unexpectedly negative."};
                        }
                    }

                    std::stringstream ss{utf8str};
                    return_type output{};
                    ss >> output;
                    return output;
                } catch (errors::unreadable_scalar& urs) {
                    throw std::move(urs); // rethrow
                } catch (std::exception& e) {
                    throw errors::unreadable_scalar{errors::could_not_convert, "Could not convert string to integer."};
                }
            }

        };

        static_assert(concepts::VisitorHasRealDense<IntReaderVisitor<long>>);
        static_assert(concepts::VisitorHasString<IntReaderVisitor<long>>);

        template<typename output_type>
        output_type read_as(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            return DispatchVisitor(engine, input, IntReaderVisitor<output_type>{});
        }

        template<typename output_type>
        output_type read_as_or_fail(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            try {
                return read_as<output_type>(engine, input);
            } catch (const errors::unreadable_scalar& e) {
                throw_error(engine, e.errCode, e.what());
            }
        }


    }


    long read_as_long(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return read_as<long>(engine, input);
    }

    long read_as_long_or_fail(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return read_as_or_fail<long>(engine, input);
    }

    unsigned long read_as_ulong(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
        return read_as<unsigned long>(engine, input);
    }

    unsigned long read_as_ulong_or_fail(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return read_as_or_fail<unsigned long>(engine, input);
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

}
