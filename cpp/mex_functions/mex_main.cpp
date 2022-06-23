/**
 * mex_main.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "mex_main.h"

#include "utilities/reporting.h"
#include "utilities/read_as_string.h"

#include "functions/mex_function.h"
#include "functions/function_list.h"

#include "storage_manager.h"

#include "MatlabEngine/engine_interface_util.hpp"
#include <sstream>

namespace NPATK::mex {
    MexMain::MexMain(std::shared_ptr <matlab::engine::MATLABEngine> matlab_engine)
        : matlabPtr(std::move(matlab_engine)), persistentStorage{getStorageManager()} {

    }

    void MexMain::operator()(IOArgumentRange outputs, IOArgumentRange inputs) {
        // Read and pop function name...
        functions::MEXEntryPointID function_id = get_function_id(inputs);
        assert(function_id != functions::MEXEntryPointID::Unknown);

        // Construction function object from ID...
        std::unique_ptr<functions::MexFunction> the_function = functions::make_mex_function(*matlabPtr,
                                                                                            function_id,
                                                                                            persistentStorage);
        if (!the_function) {
            throw_error(*matlabPtr, errors::bad_function, u"Internal error: could not create function object.");
        }

        // Get named parameters & flags
        auto processed_inputs = this->clean_inputs(*the_function, inputs);
        assert(processed_inputs);

        // Check inputs are in range, and are valid
        this->validate_inputs(*the_function, *processed_inputs);

        // Check outputs are in range
        this->validate_outputs(*the_function, outputs);

        // Pre-process universal input flags
        bool is_debug = processed_inputs->flags.contains(u"debug");
        bool is_verbose = is_debug || processed_inputs->flags.contains(u"verbose");
        bool is_quiet = processed_inputs->flags.contains(u"quiet") && !is_verbose;

        bool preprocess_only = processed_inputs->flags.contains(u"debug_preprocess");

        the_function->setQuiet(is_quiet);
        the_function->setDebug(is_debug);
        the_function->setVerbose(is_verbose);

        // Final function-specific pre-processing and validation of inputs
        processed_inputs = this->transform_and_validate(*the_function, std::move(processed_inputs), outputs);

        // If only transforming parameters, print output:
        if (preprocess_only) {
            print_to_console(*this->matlabPtr, processed_inputs->to_string());
            matlab::data::ArrayFactory factory{};
            for (auto &output: outputs) {
                output = factory.createScalar(0);
            }
            return;
        }

        // Execute function
        (*the_function)(outputs, std::move(processed_inputs));

        // ~the_function
    }

    functions::MEXEntryPointID MexMain::get_function_id(IOArgumentRange& inputs) {
        if (inputs.size() <= 0) {
            return functions::MEXEntryPointID::Version;
        }

        auto command_arg = read_as_utf16(inputs.pop_front());
        if (!command_arg.has_value()) {
            throw_error(*matlabPtr, errors::bad_function,
                        u"First argument must be a single function name (i.e. one string).");
        }

        auto entry_id = functions::which_entrypoint(command_arg.value());
        if (entry_id == functions::MEXEntryPointID::Unknown) {
            throw_error(*matlabPtr, errors::bad_function,
                        u"Function \"" + command_arg.value() + u"\" not known in NPATK.");
        }

        return entry_id;
    }


    std::unique_ptr<SortedInputs> MexMain::clean_inputs(const functions::MexFunction &func, IOArgumentRange & inputs) {
        const auto& param_names = func.ParamNames();
        const auto& func_flag_names = func.FlagNames();

        // Incorporate default flags:~
        NameSet flag_names = {u"quiet", u"verbose", u"debug", u"debug_preprocess"};
        flag_names.insert(func_flag_names.begin(), func_flag_names.end());

        std::unique_ptr<SortedInputs> sortedPtr = std::make_unique<SortedInputs>();
        assert(sortedPtr);
        SortedInputs& sorted = *sortedPtr;

        // Scan through inputs
        size_t cursor = 0;
        auto input_iter = inputs.begin();
        while (input_iter != inputs.end()) {
            auto& input = *input_iter;
            bool matched = false;

            auto maybe_string = read_as_utf16(input);
            if (maybe_string.has_value()) {
                const auto& param_flag_str = maybe_string.value();

                // First, is input a parameter?
                if (param_names.contains(param_flag_str)) {
                    if ((cursor+1) >= inputs.size()) {
                        throw_error(*this->matlabPtr, errors::bad_param,
                                                      u"Named parameter \"" + param_flag_str + u"\" was used,"
                                                      + u" but next argument (with data) is missing.");
                    } else {
                        ++input_iter;
                        auto& data = *input_iter;
                        sorted.params.emplace(param_flag_str, std::move(data));
                        matched = true;
                    }
                } else if (flag_names.contains(param_flag_str)) {
                    sorted.flags.emplace(param_flag_str);
                    matched = true;
                }
            }

            // Otherwise, input is flat, push to back of list...
            if (!matched) {
                sorted.inputs.emplace_back(std::move(input));
            }

            // Advance iterators...
            ++input_iter;
            ++cursor;
        }

        return sortedPtr;
    }

    void MexMain::validate_inputs(const functions::MexFunction &func, const SortedInputs &inputs) {

        // First check number of inputs is okay
        auto [min, max] = func.NumInputs();
        if ((inputs.inputs.size() > max) || (inputs.inputs.size() < min)) {

            // Build error message:
            std::string func_name{matlab::engine::convertUTF16StringToUTF8String(func.function_name)};
            std::stringstream ss;
            ss << "Function \"" << func_name << "\" ";
            if (min != max) {
                ss << "requires between " << min << " and " << max << " input parameters.";
            } else {
                if (min == 0) {
                    ss << "does not take an input.";
                } else {
                    ss << "requires " << min;
                    if (min != 1) {
                        ss << " input parameters.";
                    } else {
                        ss << " input parameter.";
                    }
                }
            }

            if (inputs.inputs.size() > max) {
                throw_error(*matlabPtr, errors::too_many_inputs, ss.str());
            } else {
                throw_error(*matlabPtr, errors::too_few_inputs, ss.str());
            }
        }

        // Next, check for mutual exclusion
        auto mutex_params = func.check_for_mutex(inputs);
        if (mutex_params.has_value()) {
            std::basic_stringstream<char16_t> bss;
            bss << u"Invalid argument to function \"" << func.function_name << "\": "
                << u"Cannot specify mutually exclusive parameters \"" << mutex_params->first << "\""
                << " and \"" << mutex_params->second << "\".";
            throw_error(*matlabPtr, errors::mutex_param, bss.str());
        }

    }

    void MexMain::validate_outputs(const functions::MexFunction &func, const IOArgumentRange &outputs) {
        auto [min, max] = func.NumOutputs();
        if ((outputs.size() > max) || (outputs.size() < min)) {

            // Build error message:
            std::string func_name{matlab::engine::convertUTF16StringToUTF8String(func.function_name)};
            std::stringstream ss;
            ss << "Function \"" << func_name << "\" ";
            if (min != max) {
                ss << "requires between " << min << " and " << max << " outputs.";
            } else {
                if (min == 0) {
                    ss << "does not have an output.";
                } else {
                    ss << "requires " << min;
                    if (min != 1) {
                        ss << " outputs.";
                    } else {
                        ss << " output.";
                    }
                }
            }

            if (outputs.size() > max) {
                throw_error(*matlabPtr, errors::too_many_outputs, ss.str());
            } else {
                throw_error(*matlabPtr, errors::too_few_outputs, ss.str());
            }
        }
    }

    std::unique_ptr<SortedInputs> MexMain::transform_and_validate(const functions::MexFunction& func,
                                                         std::unique_ptr<SortedInputs> inputs,
                                                         const IOArgumentRange& outputs) {
        try {
            // Call function's own custom validator
            return func.transform_inputs(std::move(inputs));

        } catch (const errors::BadInput& bie) {
            std::basic_stringstream<char16_t> bss;
            bss << u"Invalid argument to function \"" << func.function_name << "\": "
                << matlab::engine::convertUTF8StringToUTF16String(bie.what());
            throw_error(*matlabPtr, bie.errCode, bss.str());
        }
    }



}