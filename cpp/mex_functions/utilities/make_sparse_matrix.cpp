/**
 * make_sparse_matrix.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "make_sparse_matrix.h"

#include "mex.hpp"

namespace NPATK::mex {

    template<>
    matlab::data::SparseArray<double>
    make_zero_sparse_matrix(matlab::engine::MATLABEngine &engine, std::pair<size_t, size_t> dimensions) {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args{factory.createScalar(dimensions.first),
                                               factory.createScalar(dimensions.second)};

        return engine.feval(u"sparse", args);
    }

    template<>
    matlab::data::SparseArray<std::complex<double>>
    make_zero_sparse_matrix(matlab::engine::MATLABEngine &engine, std::pair<size_t, size_t> dimensions) {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args{factory.createScalar(dimensions.first),
                                              factory.createScalar(dimensions.second)};

        auto x = engine.feval(u"sparse", args);
        return engine.feval(u"complex", x);
    }

    template<>
    matlab::data::SparseArray<bool>
    make_zero_sparse_matrix(matlab::engine::MATLABEngine &engine, std::pair<size_t, size_t> dimensions) {
        matlab::data::ArrayFactory factory;
        std::vector<matlab::data::Array> args{factory.createScalar(dimensions.first),
                                              factory.createScalar(dimensions.second)};

        auto x = engine.feval(u"sparse", args);
        return engine.feval(u"logical", x);
    }

}