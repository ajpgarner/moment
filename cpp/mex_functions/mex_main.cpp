/**
 * mex_main.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "mex_main.h"

#include "helpers/reporting.h"
#include "helpers/read_as_string.h"

#include "functions/version.h"
#include "functions/make_symmetric.h"

namespace NPATK::mex {
    MexMain::MexMain(std::shared_ptr <matlab::engine::MATLABEngine> matlab_engine)
        : matlabPtr(std::move(matlab_engine))
    {

    }

    void MexMain::operator()(WrappedArgRange outputs, WrappedArgRange inputs) {

        debug_message(*matlabPtr, "Input size on entering MexMain(): " + std::to_string(inputs.size()) + "\n");
        debug_message(*matlabPtr, "Output size on entering MexMain(): " + std::to_string(outputs.size()) + "\n");


        functions::MEXEntryPointID function_id = get_function_id(inputs);

        // Pop function name from inputs, if supplied...
        if (inputs.size() >= 1) {
            inputs = WrappedArgRange(inputs.begin()+1, inputs.end());
        }

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

        // Execute function
        (*the_function)(outputs, inputs);

        // ~the_function
    }

    functions::MEXEntryPointID MexMain::get_function_id(WrappedArgRange inputs) {
        if (inputs.size() <= 0) {
            return functions::MEXEntryPointID::Version;
        }

        auto command_arg = read_as_utf16(inputs[0]);
        if (!command_arg.has_value()) {
            throw_error(*matlabPtr, "First argument must be a single function name (i.e. one string).");
        }

        auto entry_id = functions::which_entrypoint(command_arg.value());
        if (entry_id == functions::MEXEntryPointID::Unknown) {
            throw_error(*matlabPtr, u"Function \"" + command_arg.value() + u"\" not known in NPATK.");
        }
        return entry_id;
    }

}