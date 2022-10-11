/**
 * read_as_vector.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "read_as_vector.h"

#include "read_as_scalar.h"

#include "visitor.h"
#include "reporting.h"

#include <utility>

namespace NPATK::mex {


    namespace {
        template<typename value_type>
        class IntVectorReaderVisitor {

        public:
            using return_type = std::vector<value_type>;

        public:
            explicit IntVectorReaderVisitor() = default;

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @tparam datatype The data array type
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
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
                        output.template emplace_back(read_buf);
                    } catch (errors::unreadable_scalar &urs) {
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

        template<typename value_type>
        std::vector<value_type> read_as_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            return DispatchVisitor(engine, input, IntVectorReaderVisitor<value_type>{});
        }

        template<typename value_type>
        std::vector<value_type> read_as_vector_or_fail(matlab::engine::MATLABEngine &engine, const matlab::data::Array& input) {
            try {
                return read_as_vector<value_type>(engine, input);
            } catch (const errors::unreadable_vector& e) {
                throw_error(engine, e.errCode, e.what());
            }
        }
    }


    std::vector<uint64_t>
    read_as_uint64_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return read_as_vector<uint64_t>(engine, input);
    }

    std::vector<int64_t>
    read_as_int64_vector(matlab::engine::MATLABEngine &engine, const matlab::data::Array &input) {
        return read_as_vector<int64_t>(engine, input);
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