/**
 * mtk_function.h
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <concepts>
#include <memory>
#include <string>
#include <set>
#include <utility>

#include "function_list.h"
#include "utilities/io_parameters.h"
#include "utilities/reporting.h"

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
    class MTKFunction {
    protected:
        /** The MATLAB engine handle. */
        matlab::engine::MATLABEngine& matlabEngine;

        /** Reference to persistent storage. */
        StorageManager& storageManager;

        /**
         * Shared ptr to settings just before function invocation.
         * For thread safety, use this settings object within the function execution - as there is no guarantee that the
         * settings object in the storage manager is not changed part-way through execution.
         */
        std::shared_ptr<const EnvironmentalVariables> settings;

        /** Input strings that will be treated as boolean flags if set. */
        NameSet flag_names{};

        /** Input strings that will be treated as indicators that the following input is a named parameter. */
        NameSet param_names{};

        /** Handle flags and/or parameters that cannot be simultaneously defined. */
        MutuallyExclusiveParams mutex_params{};

        /** Minimum number of outputs to be supplied from MATLAB, below which the function will error. */
        size_t min_outputs = 0;
        /** Maximum number of outputs to be supplied from MATLAB, above which the function will error. */
        size_t max_outputs = 0;
        /** Minimum number of inputs to be supplied from MATLAB, below which the function will error. */
        size_t min_inputs = 0;
        /** Maximum number of inputs to be supplied from MATLAB, above which the function will error. */
        size_t max_inputs = 0;

        /** True if warnings are supressed. */
        bool quiet = false;
        /** True to display intermediate output. */
        bool verbose = false;
        /** True to display a lot of intermediate output. */
        bool debug = false;

    public:
        /** The numeric ID of the function. */
        const MTKEntryPointID function_id;

        /**
         * Constructs a function, to be invoked from MATLAB as: mtk(name, args...)
         * @param engine Handle to the MATLAB engine.
         * @param storage Reference to persistent storage manager.
         * @param id The numeric ID of the function.
         * @param name The name of the function, as it should be invoked.
         */
        MTKFunction(matlab::engine::MATLABEngine& engine, StorageManager& storage, MTKEntryPointID id);

        virtual ~MTKFunction();

        /**
         * Executes the MEX function.
         * @param output Range over MATLAB output arrays.
         * @param input Pointer to (semi-)parsed input array.
         */
        virtual void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> input) = 0;

        /**
         * Checks if parsed input array has any mutually-exclusive parameters.
         * @param input Reference to parsed input array.
         * @return Empty optional if no mutually-exclusive parameters; otherwise a pair of clashing parameter strings.
         */
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
     * Utility intermediate abstract class - binds function to appropriate parameter subclasses.
     * @tparam param_t Subclass of SortedInputs defining parameters specifically for this function.
     * @tparam i_entry_id The numeric ID of the mex function.
     */
    template<std::derived_from<SortedInputs> param_t, MTKEntryPointID i_entry_id>
    class ParameterizedMTKFunction : public MTKFunction {
    public:
        /** Subclass of SortedInputs defining parameters specifically for this mex function. */
        using parameter_type = param_t;
        /** The numeric ID of this mex function. */
        static const MTKEntryPointID entry_id = i_entry_id;

    protected:
        /**
         * Construct a mex function with additional pre-processing of the input parameters.
         * @param engine Handle to the MATLAB engine.
         * @param storage Reference to persistent storage manager.
         * @param name The name of the function, as it should be invoked.
         */
        ParameterizedMTKFunction(matlab::engine::MATLABEngine& engine, StorageManager& storage)
                 : MTKFunction(engine, storage, entry_id) { }

    public:

        [[nodiscard]] std::unique_ptr<SortedInputs>
        transform_inputs(std::unique_ptr<SortedInputs> input) const final {
            assert(input);
            auto output = std::make_unique<param_t>(std::move(*input));
            assert(output);
            try {
                extra_input_checks(*output);
            } catch (const std::exception& e) {
                throw_error(this->matlabEngine, errors::bad_param, e.what());
            }
            return output;
        }

        void operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputRaw) final {
            assert(inputRaw);
            auto& input = dynamic_cast<param_t&>(*inputRaw);
            this->operator()(std::move(output), input);
        }

    protected:
        /**
         * Execute the mex function on pre-processed parameters.
         * @param output Range over the MATLAB output arrays.
         * @param input The pre-processed input parameters.
         */
        virtual void operator()(IOArgumentRange output, param_t& input) = 0;

        /**
         * Checks if pre-processed paramaters are valid, which cannot be done when the parameters are first parsed.
         * Override, and throw exceptions if parameters invalid.
         * @param input Check pre-processed parameters are valid.
         */
        virtual void extra_input_checks(param_t& input) const { }

    };
}