/**
 * function_base.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <set>
#include <utility>

#include "functions/function_list.h"
#include "utilities/io_parameters.h"

namespace matlab::engine {
    class MATLABEngine;
}

namespace Moment::mex {
    class StorageManager;
    class EnvironmentalVariables;
}

namespace Moment::mex::functions {

    /**
     * Base class, for various mex functions called in the toolkit.
     */
    class MexFunction {
    protected:
        matlab::engine::MATLABEngine& matlabEngine;
        StorageManager& storageManager;

        /**
         * Shared ptr to settings just before function invocation.
         * For thread safety, use this settings object within the function execution - as there is no guarantee that the
         * settings object in the storage manager is not changed part-way through execution.
         */
        std::shared_ptr<const EnvironmentalVariables> settings;

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
                    MEXEntryPointID id, std::basic_string<char16_t> name);

        virtual ~MexFunction();

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
        transform_inputs(std::unique_ptr<SortedInputs> input) const;

        /**
         * Validates that the number of outputs matches that expected from the input parameters
         */
         virtual void validate_output_count(size_t outputs, const SortedInputs& inputs) const { }

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
        void setQuiet(bool val = true) noexcept;

        /**
         * Flag whether the function should output verbose information to console.
         */
        void setVerbose(bool val = true) noexcept;

        /**
         * Flag whether the function should output debug information to console.
         */
        void setDebug(bool val = true) noexcept;
    };

    /**
     * Utility intermediate abstract class - binds function to appropriate parameter types
     */
    template<std::derived_from<SortedInputs> param_t, MEXEntryPointID i_entry_id>
    class ParameterizedMexFunction : public MexFunction {
    public:
        using parameter_type = param_t;
        static const MEXEntryPointID entry_id = i_entry_id;

    protected:
        ParameterizedMexFunction(matlab::engine::MATLABEngine& engine, StorageManager& storage,
                                 std::basic_string<char16_t> name)
                 : MexFunction(engine, storage, entry_id, name) { }

    public:

        [[nodiscard]] std::unique_ptr<SortedInputs>
        transform_inputs(std::unique_ptr<SortedInputs> input) const final {
            assert(input);
            auto output = std::make_unique<param_t>(this->matlabEngine, std::move(*input));
            assert(output);
            extra_input_checks(*output);
            return output;
        }

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputRaw) final {
            assert(inputRaw);
            auto& input = dynamic_cast<param_t&>(*inputRaw);
            this->operator()(std::move(output), input);
        }

    protected:
        virtual void operator()(IOArgumentRange output, param_t& input) = 0;

        virtual void extra_input_checks(param_t& input) const { }

    };
}