/**
 * reporting.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "reporting.h"
#include "errors.h"

#include "MatlabDataArray.hpp"

namespace Moment::mex {

    void print_warning(matlab::engine::MATLABEngine &engine,
                       const std::string &message) {
        std::string warn_msg = std::string("[\bWARNING: ") + message + "]\b\n";
        print_to_console(engine, warn_msg);
    }

    void print_to_console(matlab::engine::MATLABEngine &engine,
                          const std::string &message) {
        matlab::data::ArrayFactory factory;
        engine.feval(u"fprintf",
             0, std::vector<matlab::data::Array>({ factory.createScalar(message) }));
    }

    void print_to_console(matlab::engine::MATLABEngine &engine,
                          const std::basic_string<char16_t> &message) {
        matlab::data::ArrayFactory factory;
        engine.feval(u"fprintf",
             0, std::vector<matlab::data::Array>({ factory.createScalar(message) }));
    }
}