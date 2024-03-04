/**
 * errors.h
 *
 * @copyright Copyright (c) 2022-2024 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "errors.h"

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <exception>
#include <sstream>

namespace Moment::mex {
    namespace {
        /** Error message for bad function */
        [[nodiscard]] std::string make_bad_function_msg(const std::string& unknown_func) {
            std::stringstream errSS;
            errSS << "Function \"" << unknown_func + "\" is not in the Moment library.";
            return errSS.str();
        }
    }

    BadFunctionException::BadFunctionException()
        : MomentMEXException{BadFunctionException::bad_function,
                             "First argument must be a single function name (i.e. one string)."} { }

    BadFunctionException::BadFunctionException(const std::string& func_name)
        : MomentMEXException{BadFunctionException::bad_function, make_bad_function_msg(func_name)} { }

    namespace {
        template<bool is_output>
        [[nodiscard]] std::string make_bad_count_msg(const std::string& func_name,
                                                     size_t min, size_t max, size_t actual) {
            // Build error message:
            std::stringstream ss;
            ss << "Function \"" << func_name << "\" ";
            if (min != max) {
                ss << "requires between " << min << " and " << max;
                if constexpr (is_output) {
                    ss << " outputs.";
                } else {
                    ss << " input parameters.";
                }
            } else {
                if (min == 0) {
                    ss << "does not ";
                    if constexpr (is_output) {
                        ss << "write an output.";
                    } else {
                        ss << "take an input.";
                    }
                } else {
                    ss << "requires " << min;
                    if constexpr (is_output) {
                        if (min != 1) {
                            ss << " outputs.";
                        } else {
                            ss << " output.";
                        }
                    } else {
                        if (min != 1) {
                            ss << " input parameters.";
                        } else {
                            ss << " input parameter.";
                        }
                    }
                }
            }
            return ss.str();
        }
    }

    InputCountException::InputCountException(const std::string& func_name, size_t min, size_t max, size_t actual,
                                             const std::string& what)
        : MomentMEXException{(actual > max) ? InputCountException::too_many_inputs
                                            : InputCountException::too_few_inputs,
                             what}, function_name{func_name}, min_expected{min}, max_expected{max}, actual{actual} { }

    InputCountException::InputCountException(const std::string& func_name, size_t min, size_t max, size_t actual)
            : InputCountException{func_name, min, max, actual,
                                  make_bad_count_msg<false>(func_name, min, max, actual)} { }


    OutputCountException::OutputCountException(const std::string& func_name, size_t min, size_t max, size_t actual,
                                               const std::string& what)
            : MomentMEXException{(actual > max) ? OutputCountException::too_many_outputs
                                                : OutputCountException::too_few_outputs,
                                 what}, function_name{func_name}, min_expected{min}, max_expected{max}, actual{actual} { }


    OutputCountException::OutputCountException(const std::string& func_name, size_t min, size_t max, size_t actual)
            : OutputCountException{func_name, min, max, actual,
                                   make_bad_count_msg<true>(func_name, min, max, actual)} { }



    namespace {
        [[nodiscard]] std::string make_bad_mutex_msg(const std::string& func_name,
                                                     const std::string& param1, const std::string& param2) {
            std::stringstream errSS;
            errSS << "Invalid argument to function \"" << func_name << "\": "
                  << "Cannot specify mutually exclusive parameters \"" << param1<< "\""
                  << " and \"" << param2 << "\".";
            return errSS.str();
        }
    }

    MutexParamException::MutexParamException(const std::string& func_name,
                                             const std::string& param1, const std::string& param2)
        : MomentMEXException{"mutex_param", make_bad_mutex_msg(func_name, param1, param2)},
          func_name{func_name}, param1{param1}, param2{param2} { }


    void MomentMEXException::throw_to_MATLAB(matlab::engine::MATLABEngine& engine) const {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> errParams;
        errParams.emplace_back(factory.createScalar(this->error_code));
        errParams.emplace_back(factory.createScalar(this->error_msg));
        engine.feval(u"error", 0, std::move(errParams));

        std::terminate(); // hint for compiler, should be unreachable.
    }

}