#include "reporting.h"
#include "MatlabDataArray.hpp"
//#include "MatlabDataArray/ArrayFactory.hpp"

namespace NPATK {

    void mex::throw_error(matlab::engine::MATLABEngine &engine, const std::string& error) {
        matlab::data::ArrayFactory factory;
        engine.feval(u"error",
             0, std::vector<matlab::data::Array>({ factory.createScalar(error) }));
    }

    void mex::debug_message(matlab::engine::MATLABEngine &engine, const std::string &message) {
        matlab::data::ArrayFactory factory;
        engine.feval(u"fprintf",
             0, std::vector<matlab::data::Array>({ factory.createScalar(message) }));
    }
}