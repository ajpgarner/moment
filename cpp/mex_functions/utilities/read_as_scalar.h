/**
 * read_as_scalar.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <complex>
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

    /**
     * Parse MATLAB array into bool.
     */
    [[nodiscard]] bool read_as_boolean(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into 16-bit signed integer.
     */
    [[nodiscard]] int16_t  read_as_int16(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into 16-bit unsigned integer.
     */
    [[nodiscard]] uint16_t read_as_uint16(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into 32-bit signed integer.
     */
    [[nodiscard]] int32_t  read_as_int32(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into 32-bit unsigned integer.
     */
    [[nodiscard]] uint32_t read_as_uint32(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into 64-bit signed integer.
     */
    [[nodiscard]] int64_t  read_as_int64(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into 64-bit unsigned integer.
     */
    [[nodiscard]] uint64_t read_as_uint64(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    /**
     * Parse MATLAB array into size type signed integer (should be 64-bits).
     * Specialist function required for systems where int32 = int and int64 = long long, but size_t = long.
     */
    [[nodiscard, maybe_unused]] size_t read_as_size_t(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input);

    [[nodiscard]] float read_as_float(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] double read_as_double(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    [[nodiscard]] std::complex<float> read_as_complex_float(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::complex<double> read_as_complex_double(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);

    template<std::integral int_t>
    int_t read_as_scalar(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<>
    inline bool read_as_scalar<bool>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_boolean(engine, input);
    }

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

template<std::floating_point float_t>
    float_t read_as_scalar(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<>
    inline float read_as_scalar<float>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_float(engine, input);
    };

    template<>
    inline double read_as_scalar<double>(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_double(engine, input);
    };

    template<std::floating_point float_t>
    std::complex<float_t> read_as_complex_scalar(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<>
    inline std::complex<float> read_as_complex_scalar(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_complex_float(engine, input);
    }

    template<>
    inline std::complex<double> read_as_complex_scalar(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input) {
        return read_as_complex_double(engine, input);
    }

    /**
     * Read matlab string into uint64_t
     */
    [[nodiscard]] uint64_t read_as_scalar(matlab::engine::MATLABEngine& engine,
                                          const matlab::data::MATLABString& input);

    /**
     * True if the supplied type can be interpreted as a scalar integer.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_scalar_int(const matlab::data::Array& input);

    /**
     * True if the supplied type can be interpreted as a scalar floating point.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_scalar_float(const matlab::data::Array& input);

    /**
     * True if the supplied type can be interpreted as a complex scalar floating point.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_complex_scalar_float(const matlab::data::Array& input);

    /**
    * Read integer, or throw BadInput exception.
    * @param matlabEngine Reference to engine.
    * @param paramName Parameter/input name, as will appear in failure error message.
    * @param array The array to attempt to parse as a scalar integer.
    * @param min_value The minimum acceptable value of the integer.
    * @return The parsed integer.
    */
    template<std::integral int_t>
    inline int_t read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
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
     * Specialization for size_t
     */
    template<>
    inline size_t read_positive_integer<size_t>(matlab::engine::MATLABEngine &matlabEngine,
                                         const std::string& paramName, const matlab::data::Array& array,
                                         size_t min_value) {
        if (!castable_to_scalar_int(array)) {
            errors::throw_not_castable_to_scalar(paramName);
        }
        try {
            const size_t val = read_as_size_t(matlabEngine, array);
            if (val < min_value) {
                errors::throw_under_min_scalar(paramName, static_cast<size_t>(min_value));
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
    inline int_t read_positive_integer(matlab::engine::MATLABEngine &matlabEngine,
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
