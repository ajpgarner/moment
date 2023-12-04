/**
 * read_as_vector.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <concepts>
#include <stdexcept>
#include <vector>

namespace Moment::mex {

    namespace errors {
        /** Exception thrown by failed read_as_[...] functions */
        class unreadable_vector : public std::runtime_error {
        public:
            std::string errCode;
        public:
            explicit unreadable_vector(std::string errCode, const std::string &what)
                    : std::runtime_error(what), errCode{std::move(errCode)} {}
        };

        [[noreturn]] void throw_unreadable_vector(const std::string &paramName,
                                                         const unreadable_vector& urv);

        [[noreturn]] void throw_not_castable_to_vector(const std::string& paramName);

        [[noreturn]] void throw_under_min_vector(const std::string &paramName, uint64_t min_value);
    }

    [[nodiscard]] std::vector<int16_t> read_as_int16_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<uint16_t> read_as_uint16_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<int32_t> read_as_int32_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<uint32_t> read_as_uint32_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<int64_t> read_as_int64_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<uint64_t> read_as_uint64_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<float> read_as_float_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);
    [[nodiscard]] std::vector<double> read_as_double_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);


    template<std::integral int_t = uint64_t>
    [[nodiscard]] std::vector<int_t>
    read_as_vector(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<>
    [[nodiscard]] inline std::vector<int16_t> read_as_vector(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input) {
        return read_as_int16_vector(engine, input);
    }

    template<>
    [[nodiscard]] inline std::vector<uint16_t> read_as_vector(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input) {
        return read_as_uint16_vector(engine, input);
    }
    
    template<>
    [[nodiscard]] inline std::vector<int32_t> read_as_vector(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input) {
        return read_as_int32_vector(engine, input);
    }

    template<>
    [[nodiscard]] inline std::vector<uint32_t> read_as_vector(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input) {
        return read_as_uint32_vector(engine, input);
    }
    
    template<>
    [[nodiscard]] inline std::vector<int64_t> read_as_vector(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input) {
        return read_as_int64_vector(engine, input);
    }

    template<>
    [[nodiscard]] inline std::vector<uint64_t> read_as_vector(matlab::engine::MATLABEngine& engine,
                                                      const matlab::data::Array& input) {
        return read_as_uint64_vector(engine, input);
    }

    template<std::floating_point float_t>
    inline std::vector<float_t> read_as_vector(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    template<>
    inline std::vector<float> read_as_vector<float>(matlab::engine::MATLABEngine& engine,
                                                    const matlab::data::Array& input) {
        return read_as_float_vector(engine, input);
    };

    template<>
    inline std::vector<double>  read_as_vector<double>(matlab::engine::MATLABEngine& engine,
                                                       const matlab::data::Array& input) {
        return read_as_double_vector(engine, input);
    };

    /**
     * True if the supplied type can be interpreted as a vector integer.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_vector_int(const matlab::data::Array& input);

    /**
    * Read integer array, or throw BadInput exception.
    * @param matlabEngine Reference to engine.
    * @param paramName Parameter/input name, as will appear in failure error message.
    * @param array The array to attempt to parse as a scalar integer.
    * @param min_value The minimum acceptable value of the integer.
    * @return The parsed vector of integers.
    */
    template<std::integral int_t>
    inline std::vector<int_t> read_positive_integer_array(matlab::engine::MATLABEngine &matlabEngine,
                                                   const std::string& paramName, const matlab::data::Array& array,
                                                   int_t min_value = 0)  {
        if (!castable_to_vector_int(array)) {
            errors::throw_not_castable_to_vector(paramName);
        }

        try {
            auto vec = read_as_vector<int_t>(matlabEngine, array);
            for (const auto& val : vec) {
                if (val < min_value) {
                    errors::throw_under_min_vector(paramName, min_value);
                }
            }
            return vec;
        } catch (const errors::unreadable_vector& use) {
            errors::throw_unreadable_vector(paramName, use);
        }
    }

    /**
     * Read integer array, or throw BadInput exception.
     * @param matlabEngine Reference to engine.
     * @param paramName Parameter/input name, as will appear in failure error message.
     * @param array The array to attempt to parse as a scalar integer.
     * @param min_value The minimum acceptable value of the integer.
     * @return The parsed integer.
     */
    template<std::integral int_t>
    inline std::vector<int_t> read_integer_array(matlab::engine::MATLABEngine &matlabEngine,
                                                 const std::string& paramName, const matlab::data::Array& array) {

        if (!castable_to_vector_int(array)) {
            errors::throw_not_castable_to_vector(paramName);
        }
        try {
            return read_as_vector<int_t>(matlabEngine, array);
        } catch (const errors::unreadable_vector& use) {
            errors::throw_unreadable_vector(paramName, use);
        }
    }

}