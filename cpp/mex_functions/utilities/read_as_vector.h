/**
 * read_as_vector.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

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


    }

    [[nodiscard]] std::vector<int64_t> read_as_int64_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);

    [[nodiscard]] std::vector<uint64_t> read_as_uint64_vector(matlab::engine::MATLABEngine& engine,
                                                              const matlab::data::Array& input);

    /**
     * True if the supplied type can be interpreted as a vector integer.
     * @param input The matlab array object to test.
     */
    [[nodiscard]] bool castable_to_vector_int(const matlab::data::Array& input);


}