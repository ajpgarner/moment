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
#include <map>

namespace NPATK::mex {

    template<typename data_t>
    using sparse_set_build = std::map<std::pair<size_t, size_t>, data_t>;

    template<typename data_t>
    inline matlab::data::SparseArray<data_t> make_sparse_matrix(matlab::data::ArrayFactory& factory,
                                                         std::pair<size_t, size_t> dimensions,
                                                         const std::vector<size_t>& rows,
                                                         const std::vector<size_t>& cols,
                                                         const std::vector<data_t>& values) {

            matlab::data::ArrayDimensions dim{dimensions.first, dimensions.second};

            size_t nnz = values.size();
            assert(rows.size() == nnz);
            assert(cols.size() == nnz);

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

    template<typename data_t>
    inline matlab::data::SparseArray<data_t> make_sparse_matrix(matlab::data::ArrayFactory& factory,
                                                                std::pair<size_t, size_t> dimensions,
                                                                const sparse_set_build<data_t>& specification) {

        matlab::data::ArrayDimensions dim{dimensions.first, dimensions.second};

        size_t nnz = specification.size();

        auto rows_p = factory.createBuffer<size_t>(nnz);
        auto cols_p = factory.createBuffer<size_t>(nnz);
        auto data_p = factory.createBuffer<data_t>(nnz);

        // Write data into the buffers
        data_t* dataPtr = data_p.get();
        size_t* rowsPtr = rows_p.get();
        size_t* colsPtr = cols_p.get();
        for (auto [indices, value] : specification) {
            *(rowsPtr++) = indices.first;
            *(colsPtr++) = indices.second;
            *(dataPtr++) = value;
        }

        return factory.createSparseArray<data_t>(dim, nnz,
                                                 std::move(data_p), std::move(rows_p), std::move(cols_p));
    }

    /**
     * Copies a sparse array into a map structure, for random access purposes.
     * @tparam input_type The data type in the MATLAB array
     * @tparam output_type The data type in the map
     * @param inputArray The matlab sparse array
     * @return A map containing the data.
     */
    template<typename input_type, typename output_type = input_type>
            requires std::convertible_to<input_type, output_type>
    sparse_set_build<output_type>
    sparse_array_to_map(const matlab::data::SparseArray<input_type>& inputArray) {
        sparse_set_build<output_type> output{};
        for (auto iter = inputArray.cbegin(); iter != inputArray.cend(); ++iter) {
            auto indices = inputArray.getIndex(iter);
            auto value = static_cast<output_type>(*iter);
            if (value != 0) {
                output.insert(output.end(), {indices, value});
            }
        }
        return output;
    }
}
