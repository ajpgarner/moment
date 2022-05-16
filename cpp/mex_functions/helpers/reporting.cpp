/**
 * reporting.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "reporting.h"
#include "MatlabDataArray.hpp"

namespace NPATK::mex {

    void throw_error(matlab::engine::MATLABEngine &engine, const std::string& error) {
        matlab::data::ArrayFactory factory;
        engine.feval(u"error",
             0, std::vector<matlab::data::Array>({ factory.createScalar(error) }));
    }

    void debug_message(matlab::engine::MATLABEngine &engine, const std::string &message) {
        matlab::data::ArrayFactory factory;
        engine.feval(u"fprintf",
             0, std::vector<matlab::data::Array>({ factory.createScalar(message) }));
    }
}