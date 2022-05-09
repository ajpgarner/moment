#pragma once

#include <string>

#include "mex.hpp"
#include "MatlabDataArray.hpp"

namespace NPATK {
    namespace mex {
        void throw_error(matlab::engine::MATLABEngine& engine, const std::string& error);
        void debug_message(matlab::engine::MATLABEngine& engine, const std::string& message);
    }
}