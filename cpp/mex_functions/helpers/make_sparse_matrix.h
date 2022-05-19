/**
 * make_sparse_matrix.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "MatlabDataArray/SparseArray.hpp"
#include "MatlabDataArray/ArrayFactory.hpp"

#include <cassert>
#include <complex>

namespace NPATK::mex {

    template<typename data_t>
    inline matlab::data::SparseArray<data_t> make_sparse_matrix(std::pair<size_t, size_t> dimensions,
                                                         const std::vector<size_t>& rows,
                                                         const std::vector<size_t>& cols,
                                                         const std::vector<data_t>& values) {

            matlab::data::ArrayDimensions dim{dimensions.first, dimensions.second};

            size_t nnz = values.size();
            assert(rows.size() == nnz);
            assert(cols.size() == nnz);

            matlab::data::ArrayFactory factory;

            auto rows_p = factory.createBuffer<size_t>(rows.size());
            auto cols_p = factory.createBuffer<size_t>(cols.size());
            auto data_p = factory.createBuffer<data_t>(values.size());

            // Write data into the buffers
            data_t* dataPtr = data_p.get();
            size_t* rowsPtr = rows_p.get();
            size_t* colsPtr = cols_p.get();
            std::for_each(values.begin(), values.end(), [&](const data_t& e) { *(dataPtr++) = e; });
            std::for_each(rows.begin(), rows.end(), [&](const size_t& e) { *(rowsPtr++) = e; });
            std::for_each(cols.begin(), cols.end(), [&](const size_t& e) { *(colsPtr++) = e; });

            return factory.createSparseArray<data_t>(dim, nnz,
                                                     std::move(data_p), std::move(rows_p), std::move(cols_p));
    }
}
