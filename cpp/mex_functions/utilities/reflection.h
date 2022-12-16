/**
 * reflection.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <complex>
#include <string>

#include "MatlabDataArray.hpp"

namespace Moment::mex {
    /**
     * Get a string describing the array type.
     */
    [[nodiscard]] std::string to_string(const matlab::data::ArrayType& theType);

    /**
     * Get a string describing the array type.
     */
    [[nodiscard]] inline std::string type_as_string(const matlab::data::Array& array) {
        return to_string(array.getType());
    }

    /**
     * Get a string describing the array's type and dimensions.
     */
    [[nodiscard]] std::string summary_string(const matlab::data::Array& array);

    template<typename type>
    constexpr matlab::data::ArrayType cpptype_to_arraytype(const type& /*unused*/) {
        return matlab::data::ArrayType::UNKNOWN;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<double>(const double& /*unused*/) {
        return matlab::data::ArrayType::DOUBLE;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<float>(const float& /*unused*/) {
        return matlab::data::ArrayType::SINGLE;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<int64_t>(const int64_t& /*unused*/) {
        return matlab::data::ArrayType::INT64;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<int32_t>(const int32_t& /*unused*/) {
        return matlab::data::ArrayType::INT32;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<int16_t>(const int16_t& /*unused*/) {
        return matlab::data::ArrayType::INT16;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<int8_t>(const int8_t& /*unused*/) {
        return matlab::data::ArrayType::INT8;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<uint64_t>(const uint64_t& /*unused*/) {
        return matlab::data::ArrayType::UINT64;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<uint32_t>(const uint32_t& /*unused*/) {
        return matlab::data::ArrayType::UINT32;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<uint16_t>(const uint16_t& /*unused*/) {
        return matlab::data::ArrayType::UINT16;
    }

    template<>
    constexpr matlab::data::ArrayType cpptype_to_arraytype<uint8_t>(const uint8_t& /*unused*/) {
        return matlab::data::ArrayType::UINT8;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<double>>(const std::complex<double>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_DOUBLE;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<float>>(const std::complex<float>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_SINGLE;
    }
    
    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<int64_t>>(const std::complex<int64_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_INT64;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<int32_t>>(const std::complex<int32_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_INT32;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<int16_t>>(const std::complex<int16_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_INT16;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<int8_t>>(const std::complex<int8_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_INT8;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<uint64_t>>(const std::complex<uint64_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_UINT64;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<uint32_t>>(const std::complex<uint32_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_UINT32;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<uint16_t>>(const std::complex<uint16_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_UINT16;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<std::complex<uint8_t>>(const std::complex<uint8_t>& /*unused*/) {
        return matlab::data::ArrayType::COMPLEX_UINT8;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<matlab::data::MATLABString>(const matlab::data::MATLABString& /*unused*/) {
        return matlab::data::ArrayType::MATLAB_STRING;
    }

    template<>
    constexpr matlab::data::ArrayType
    cpptype_to_arraytype<char>(const char& /*unused*/) {
        return matlab::data::ArrayType::CHAR;
    }



}