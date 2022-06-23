/**
 * function_base.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <memory>
#include <string>
#include <set>
#include <utility>

#include "function_list.h"
#include "utilities/io_parameters.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace NPATK::mex {
    class StorageManager;
}

namespace NPATK::mex::functions {

    /**
     * Base class, for various mex functions called in the toolkit.
     */
    class MexFunction {
    protected:
        matlab::engine::MATLABEngine& matlabEngine;
        StorageManager& storageManager;

        NameSet flag_names{};
        NameSet param_names{};
        MutuallyExclusiveParams mutex_params{};

        size_t min_outputs = 0;
        size_t max_outputs = 0;
        size_t min_inputs = 0;
        size_t max_inputs = 0;

        /** True if warnings are supressed */
        bool quiet = false;
        /** True to display intermediate output */
        bool verbose = false;
        /** True to display a lot of output */
        bool debug = false;

    public:
        const MEXEntryPointID function_id;
        const std::basic_string<char16_t> function_name;

        MexFunction(matlab::engine::MATLABEngine& engine, StorageManager& storage,
                    MEXEntryPointID id, std::basic_string<char16_t> name)
            : matlabEngine(engine), storageManager{storage}, function_id{id}, function_name{std::move(name)} { }

        virtual ~MexFunction() = default;

        virtual void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) = 0;

        [[nodiscard]] inline auto check_for_mutex(const SortedInputs& input) const {
            return mutex_params.validate(input.flags, input.params);
        }

        /**
         * Validates that inputs are correct, and restructure them as necessary.
         * @param input Owning pointer to input object
         * @return Owning pointer to (possibly transformed) input object.
         * @throws error::BadInput If input cannot be transformed for any reason.
         */
        [[nodiscard]] virtual std::unique_ptr<SortedInputs>
        transform_inputs(std::unique_ptr<SortedInputs> input) const {
            return std::move(input);
        }

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

        /**
         * Get the range of (non-named) inputs expected
         * @return Pair, first: minimum number of inputs, second: maximum number of inputs.
         */
        [[nodiscard]] constexpr std::pair<size_t, size_t> NumInputs() const noexcept {
            return {min_inputs, max_inputs};
        }

        /**
          * Flag whether the function should supress warning messages
          */
        constexpr void setQuiet(bool val = true) noexcept {
            // Quiet mode only turns on if debug mode not set.
            this->quiet = val && !this->debug;
            if (val) {
                // Turning on quiet mode turns off verbose mode
                this->verbose = false;
            }
        }

        /**
         * Flag whether the function should output verbose information to console.
         */
        constexpr void setVerbose(bool val = true) noexcept {
            this->verbose = val;
            if (val) {
                // Turning on verbosity turns off quiet mode
                this->quiet = false;
            } else {
                // Turning off verbosity also turns off debug mode
                this->debug = false;
            }
        }

        /**
         * Flag whether the function should output debug information to console.
         */
        constexpr void setDebug(bool val = true) noexcept {
            this->debug = val;
            if (val) {
                // Turning on debug mode turns on verbosity, and turns off quiet mode
                this->verbose = true;
                this->quiet = false;
            }
        }



    };
}