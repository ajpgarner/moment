/**
 * version.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "function_base.h"
#include "version.h"

#include "../helpers/reporting.h"

namespace NPATK::mex::functions {


    Version::Version(matlab::engine::MATLABEngine &matlabEngine)
        : MexFunction(matlabEngine, MEXEntryPointID::Version, u"version") {
        this->max_outputs = 1;
        this->flag_names.emplace(u"structured");
    }

    void Version::operator()(FlagArgumentRange output, SortedInputs&& input) {
        size_t num_outputs = output.size();
        if ((0 == num_outputs) || input.flags.contains(u"verbose")) {
            debug_message(this->matlabEngine,
                          "NPA toolkit, v0.1\nCopyright (c) 2022 Austrian Academy of Sciences\n\n");
        }

        if (num_outputs >= 1) {
            matlab::data::ArrayFactory factory;
            if (input.flags.contains(u"structured")) {
                auto s = factory.createStructArray({1,1}, {"major", "minor", "build"});
                s[0]["major"] = factory.createCharArray("0");
                s[0]["minor"] = factory.createCharArray("1");
                s[0]["build"] = factory.createCharArray("x");
                output[0] = std::move(s);
            } else {
                output[0] = factory.createCharArray("0.1");
            }
        }
    }
}
