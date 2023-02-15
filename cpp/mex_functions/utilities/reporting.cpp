/**
 * reporting.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "reporting.h"
#include "error_codes.h"
#include "MatlabDataArray.hpp"

namespace Moment::mex {

    [[noreturn]] void throw_error(matlab::engine::MATLABEngine &engine,
                                  const std::string& err_code, const std::string& error) {
        matlab::data::ArrayFactory factory;
        std::string final_code{errors::applyPrefix(err_code)};
        engine.feval(u"error", 0,
             std::vector<matlab::data::Array>({factory.createScalar(final_code), factory.createScalar(error) }));
        throw; // hint for compiler
    }

    [[noreturn]] void throw_error(matlab::engine::MATLABEngine &engine,
                                  const std::string& err_code, const std::basic_string<char16_t>& error) {
        matlab::data::ArrayFactory factory;
        std::string final_code{errors::applyPrefix(err_code)};
        engine.feval(u"error", 0,
             std::vector<matlab::data::Array>({factory.createScalar(final_code), factory.createScalar(error) }));
        throw; // hint for compiler
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