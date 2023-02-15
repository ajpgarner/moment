/**
 * make_sparse_matrix.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "make_sparse_matrix.h"

#include "mex.hpp"

namespace Moment::mex {

    template<>
    matlab::data::SparseArray<double>
    make_zero_sparse_matrix(matlab::engine::MATLABEngine &engine, std::pair<size_t, size_t> dimensions) {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args{factory.createScalar(static_cast<double>(dimensions.first)),
                                               factory.createScalar(static_cast<double>(dimensions.second))};

        return engine.feval(u"sparse", args);
    }

    template<>
    matlab::data::SparseArray<std::complex<double>>
    make_zero_sparse_matrix(matlab::engine::MATLABEngine &engine, std::pair<size_t, size_t> dimensions) {
        matlab::data::ArrayFactory factory;
        
        auto sparse_template = engine.feval(u"sparse", factory.createArray<std::complex<double>>({1, 1}, {0}));

        std::vector<matlab::data::Array> args{factory.createScalar(static_cast<double>(dimensions.first)),
                                              factory.createScalar(static_cast<double>(dimensions.second)),
                                              factory.createCharArray("like"),
                                              sparse_template};

        return engine.feval(u"zeros", args);
    }

    template<>
    matlab::data::SparseArray<bool>
    make_zero_sparse_matrix(matlab::engine::MATLABEngine &engine, std::pair<size_t, size_t> dimensions) {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args{factory.createScalar(static_cast<double>(dimensions.first)),
                                              factory.createScalar(static_cast<double>(dimensions.second))};

        auto x = engine.feval(u"sparse", args);
        return engine.feval(u"logical", x);
    }

}