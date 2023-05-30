/**
 * version.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "version.h"

#include "integer_types.h"

#include "utilities/multithreading.h"

#include "utilities/reporting.h"

#include <sstream>

namespace Moment::mex::functions {


    Version::Version(matlab::engine::MATLABEngine &matlabEngine, StorageManager& storage)
        : MexFunction{matlabEngine, storage, MEXEntryPointID::Version} {
        this->max_outputs = 1;
        this->flag_names.emplace(u"structured");

        // DEBUG for mutual exclusion
        if constexpr(debug_mode) {
            this->flag_names.emplace(u"foo");
            this->flag_names.emplace(u"bar");
            this->param_names.emplace(u"cake");
            this->mutex_params.add_mutex(u"bar", u"foo");
            this->mutex_params.add_mutex(u"foo", u"cake");

            this->flag_names.emplace(u"alice");
            this->flag_names.emplace(u"bob");
            this->flag_names.emplace(u"charlie");
            this->mutex_params.add_mutex({u"alice", u"bob", u"charlie"});
        }
    }

    void Version::operator()(IOArgumentRange output, std::unique_ptr<SortedInputs> inputPtr) {
        size_t num_outputs = output.size();
        if ((0 == num_outputs) || this->verbose) {
            std::stringstream ss;
            ss << Moment::version::PROJECT_NAME << ", "
               << "v" << Moment::version::VERSION_MAJOR
               << "." << Moment::version::VERSION_MINOR
               << "." << Moment::version::VERSION_PATCH << "\n";
            ss << Moment::version::PROJECT_COPYRIGHT << "\n";

            if (this->debug) {
                ss << "Maximum worker threads: " << Multithreading::get_max_worker_threads() << "\n";
            }

            ss << "\n";

            print_to_console(this->matlabEngine, ss.str());
        }

        if (num_outputs >= 1) {
            matlab::data::ArrayFactory factory;
            if (inputPtr->flags.contains(u"structured")) {
                auto s = factory.createStructArray({1,1}, {"major", "minor", "patch"});
                s[0]["major"] = factory.createArray<int64_t>({ 1, 1 }, { Moment::version::VERSION_MAJOR });
                s[0]["minor"] = factory.createArray<int64_t>({ 1, 1 }, { Moment::version::VERSION_MINOR });
                s[0]["patch"] = factory.createArray<int64_t>({ 1, 1 }, { Moment::version::VERSION_PATCH });
                output[0] = std::move(s);
            } else {
                std::stringstream ss;
                ss << Moment::version::VERSION_MAJOR
                   << "." << Moment::version::VERSION_MINOR
                   << "." << Moment::version::VERSION_PATCH;
                output[0] = factory.createCharArray(ss.str());
            }
        }
    }
}
