/**
 * function_base.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <string>
#include <set>
#include <utility>

#include "function_list.h"
#include "../io_parameters.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex::functions {

    /**
     * Base class, for various mex functions called in the toolkit.
     */
    class MexFunction {
    protected:
        matlab::engine::MATLABEngine& matlabEngine;
        NameSet flag_names{};

        NameSet param_names{};

        size_t min_outputs = 0;

        size_t max_outputs = 0;

    public:
        const MEXEntryPointID function_id;
        const std::basic_string<char16_t> function_name;

        MexFunction(matlab::engine::MATLABEngine& engine, MEXEntryPointID id, std::basic_string<char16_t> name)
            : matlabEngine(engine), function_id{id}, function_name{std::move(name)} { }

        virtual ~MexFunction() = default;

        virtual void operator()(FlagArgumentRange output, SortedInputs&& input) = 0;

        /**
         * Set of allowed monadic flags for this function (e.g. "verbose")
         */
        [[nodiscard]] constexpr const NameSet& FlagNames() const noexcept { return this->flag_names; }


        /**
         * Set of allowed names of named parameters this function
         */
        [[nodiscard]] constexpr const NameSet& ParamNames() const noexcept { return this->param_names; }

        /**
         * Get the range of outputs expected
         * @return Pair, first: minimum number of outputs, second: maximum number of outputs.
         */
        [[nodiscard]] constexpr std::pair<size_t, size_t> NumOutputs() const noexcept {
            return {min_outputs, max_outputs};
        }


    };
}