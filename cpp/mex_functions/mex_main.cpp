/**
 * mex_main.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "mex_main.h"
#include "MatlabEngine/engine_interface_util.hpp"

#include "helpers/reporting.h"
#include "helpers/read_as_string.h"

#include "functions/function_base.h"
#include "functions/version.h"
#include "functions/make_symmetric.h"



#include <sstream>

namespace NPATK::mex {
    MexMain::MexMain(std::shared_ptr <matlab::engine::MATLABEngine> matlab_engine)
        : matlabPtr(std::move(matlab_engine))
    {

    }

    void MexMain::operator()(FlagArgumentRange outputs, FlagArgumentRange inputs) {
        // Read and pop function name...
        functions::MEXEntryPointID function_id = get_function_id(inputs);

        // Construction function object from ID...
        std::unique_ptr<functions::MexFunction> the_function = get_function(function_id);

        // Check outputs are in range
        this->validate_outputs(*the_function, outputs);

        // Get named parameters & flags
        auto processed_inputs = this->clean_inputs(*the_function, inputs);

        // Execute function
        (*the_function)(outputs, std::move(processed_inputs));

        // ~the_function
    }

    functions::MEXEntryPointID MexMain::get_function_id(FlagArgumentRange& inputs) {
        if (inputs.size() <= 0) {
            return functions::MEXEntryPointID::Version;
        }

        auto command_arg = read_as_utf16(inputs.pop_front());
        if (!command_arg.has_value()) {
            throw_error(*matlabPtr, "First argument must be a single function name (i.e. one string).");
        }

        auto entry_id = functions::which_entrypoint(command_arg.value());
        if (entry_id == functions::MEXEntryPointID::Unknown) {
            throw_error(*matlabPtr, u"Function \"" + command_arg.value() + u"\" not known in NPATK.");
        }

        return entry_id;
    }

    std::unique_ptr<functions::MexFunction> MexMain::get_function(functions::MEXEntryPointID function_id) {
        std::unique_ptr<functions::MexFunction> the_function;

        switch(function_id) {
            case functions::MEXEntryPointID::MakeSymmetric:
                the_function = std::make_unique<functions::MakeSymmetric>(*matlabPtr);
                break;
            case functions::MEXEntryPointID::MakeHermitian:
                the_function = std::make_unique<functions::MakeSymmetric>(*matlabPtr);
                break;
            default:
            case functions::MEXEntryPointID::Unknown:
            case functions::MEXEntryPointID::Version:
                the_function = std::make_unique<functions::Version>(*matlabPtr);
                break;
        }
        return the_function;
    }

    SortedInputs MexMain::clean_inputs(const functions::MexFunction &func, FlagArgumentRange & inputs) {
        const auto& param_names = func.ParamNames();
        const auto& func_flag_names = func.FlagNames();

        // Incorporate default flags:~
        NameSet flag_names = {u"verbose", u"debug"};
        flag_names.insert(func_flag_names.begin(), func_flag_names.end());

        SortedInputs sorted{};

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
                        throw_error(*this->matlabPtr, u"Named parameter \"" + param_flag_str + u"\" was used,"
                                                      + u" but next argument (with data) is missing.");
                        throw;
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

        return sorted;
    }

    void MexMain::validate_outputs(const functions::MexFunction &func, const FlagArgumentRange &outputs) {
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

            throw_error(*matlabPtr, ss.str());
        }
    }
}