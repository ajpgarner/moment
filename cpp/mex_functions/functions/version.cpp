/**
 * version.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "function_base.h"
#include "version.h"

#include "../helpers/reporting.h"

#include <sstream>

namespace NPATK::mex::functions {


    Version::Version(matlab::engine::MATLABEngine &matlabEngine)
        : MexFunction(matlabEngine, MEXEntryPointID::Version, u"version") {
        this->max_outputs = 1;
        this->flag_names.emplace(u"structured");
    }

    void Version::operator()(FlagArgumentRange output, SortedInputs&& input) {
        size_t num_outputs = output.size();
        if ((0 == num_outputs) || input.flags.contains(u"verbose")) {
            std::stringstream ss;
            ss << NPATK::version::PROJECT_NAME << ", "
               << "v" << NPATK::version::VERSION_MAJOR
               << "." << NPATK::version::VERSION_MINOR
               << "." << NPATK::version::VERSION_BUILD << "\n";
            ss << NPATK::version::PROJECT_COPYRIGHT << "\n\n";

            print_to_console(this->matlabEngine, ss.str());
        }

        if (num_outputs >= 1) {
            matlab::data::ArrayFactory factory;
            if (input.flags.contains(u"structured")) {
                auto s = factory.createStructArray({1,1}, {"major", "minor", "build"});
                s[0]["major"] = factory.createArray<int64_t>({ 1, 1 }, { NPATK::version::VERSION_MAJOR });
                s[0]["minor"] = factory.createArray<int64_t>({ 1, 1 }, { NPATK::version::VERSION_MINOR });
                s[0]["build"] = factory.createArray<int64_t>({ 1, 1 }, { NPATK::version::VERSION_BUILD });
                output[0] = std::move(s);
            } else {
                std::stringstream ss;
                ss << NPATK::version::VERSION_MAJOR
                   << "." << NPATK::version::VERSION_MINOR
                   << "." << NPATK::version::VERSION_BUILD;
                output[0] = factory.createCharArray(ss.str());
            }
        }
    }
}
