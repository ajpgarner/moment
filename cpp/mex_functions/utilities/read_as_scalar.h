/**
 * read_as_int.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <optional>
#include <utility>


namespace NPATK::mex {

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
    }

    [[nodiscard]] int64_t  read_as_int16(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint16(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] int64_t  read_as_int32(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint32(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] int64_t  read_as_int64(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint64(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);

    [[nodiscard]] uint64_t read_as_uint64(matlab::engine::MATLABEngine& engine,
                                          const matlab::data::MATLABString& input);

    [[nodiscard]] int64_t  read_as_int16_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint16_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] int64_t  read_as_int32_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint32_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] int64_t  read_as_int64_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);
    [[nodiscard]] uint64_t read_as_uint64_or_fail(matlab::engine::MATLABEngine& engine, const matlab::data::Array& input);




    /**
     * True if the supplied type can be interpreted as a scalar integer.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_scalar_int(const matlab::data::Array& input);



}


