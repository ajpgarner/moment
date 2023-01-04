/**
 * read_as_scalar.h
 * 
 * Copyright (c) 2022-2023 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <optional>
#include <utility>

namespace Moment::mex {

    namespace errors {
        /** Error code: thrown when array is unexpectedly empty */
        constexpr char empty_array[] = "empty_array";

        /** Error code: thrown when array is unexpectedly not a scalar */
        constexpr char not_a_scalar[] = "not_a_scalar";

        /** Error code: thrown when conversion is not possible */
        constexpr char could_not_convert[] = "could_not_convert";

        /** Error code: thrown when value is unexpectedly negative */
        constexpr char negative_value[] = "negative_value";


        /** Exception thrown by failed read_as_[...] functions */
        class unreadable_scalar : public std::runtime_error {
        public:
            std::string errCode;
        public:
            explicit unreadable_scalar(std::string errCode, const std::string &what)
                    : std::runtime_error(what), errCode{std::move(errCode)} {}
        };

        /** Throws a formatted bad input exception */
        [[noreturn]] void throw_unreadable_scalar(const std::string &paramName,
                                                         const unreadable_scalar& urs);

        /** Throws a formatted bad input exception */
        [[noreturn]] void throw_not_castable_to_scalar(const std::string &paramName);

        /** Throws a formatted bad input exception */
        [[noreturn]] void throw_under_min_scalar(const std::string &paramName, int64_t min_value);

        /** Throws a formatted bad input exception */
        [[noreturn]] void throw_over_max_scalar(const std::string &paramName, uint64_t max_value);

    }

    [[nodiscard]] int16_t  read_as_int16(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint16_t read_as_uint16(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] int32_t  read_as_int32(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint32_t read_as_uint32(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] int64_t  read_as_int64(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint64(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<std::integral int_t>
    inline int_t read_as_scalar(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<>
    inline int16_t read_as_scalar<int16_t>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_int16(engine, input);
    };

    template<>
    inline uint16_t read_as_scalar<uint16_t>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_uint16(engine, input);
    };

    template<>
    inline int32_t read_as_scalar<int32_t>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_int32(engine, input);
    };

    template<>
    inline uint32_t read_as_scalar<uint32_t>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_uint32(engine, input);
    };

    template<>
    inline int64_t read_as_scalar<int64_t>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_int64(engine, input);
    };

    template<>
    inline uint64_t read_as_scalar<uint64_t>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_uint64(engine, input);
    };

    [[nodiscard]] uint64_t read_as_scalar(matlab::engine::MATLABEngine& engine,
                                          const matlab::data::MATLABString& input);

    /**
     * True if the supplied type can be interpreted as a scalar integer.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_scalar_int(const matlab::data::Array& input);

    /**
    * Read integer, or throw BadInput exception.
    * @param matlabEngine Reference to engine.
    * @param paramName Parameter/input name, as will appear in failure error message.
    * @param array The array to attempt to parse as a scalar integer.
    * @param min_value The minimum acceptable value of the integer.
    * @return The parsed integer.
    */
    template<std::integral int_t>
    int_t read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
                                const std::string& paramName, const matlab::data::Array& array,
                                int_t min_value = static_cast<int_t>(0)) {
        if (!castable_to_scalar_int(array)) {
            errors::throw_not_castable_to_scalar(paramName);
        }
        try {
            auto val = read_as_scalar<int_t>(matlabEngine, array);
            if (val < min_value) {
                errors::throw_under_min_scalar(paramName, static_cast<int64_t>(min_value));
            }
            return val;
        } catch (const errors::unreadable_scalar& use) {
            errors::throw_unreadable_scalar(paramName, use);
        }
    }

    /**
     * Read integer, or throw BadInput exception.
     * @param matlabEngine Reference to engine.
     * @param paramName Parameter/input name, as will appear in failure error message.
     * @param array The array to attempt to parse as a scalar integer.
     * @param min_value The minimum acceptable value of the integer.
     * @return The parsed integer.
     */
    template<std::integral int_t>
    static int_t read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
                                       const std::string& paramName, const matlab::data::MATLABString& mlString,
                                       const int_t min_value = 0) {
        const auto max_value = static_cast<uint64_t>(std::numeric_limits<int_t>::max());
        try {
            uint64_t val = read_as_scalar(matlabEngine, mlString);
            if (val < min_value) {
                errors::throw_under_min_scalar(paramName, min_value);
            }
            if (val > max_value) {
                errors::throw_over_max_scalar(paramName, max_value);
            }
            return static_cast<int_t>(val);
        } catch (const errors::unreadable_scalar& use) {
            errors::throw_unreadable_scalar(paramName, use);
        }
    }
}
