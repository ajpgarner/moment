/**
 * impose_symmetric_constraints.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "mex.hpp"
#include "mexAdapter.hpp"

#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include "helpers/reporting.h"
#include "symbol.h"
#include "symbol_set.h"
#include "symbol_tree.h"


namespace NPATK::mex {
    /**
     * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
     * @tparam datatype The data array type
     * @param data The data array
     * @return A vector of non-matching elements, in canonical form.
     */
    template<typename datatype>
    std::vector<SymbolPair> identify_nonsymmetric_elements_dense(const matlab::data::TypedArray<datatype>& data) {
        std::vector<SymbolPair> output{};

        size_t dimension = data.getDimensions()[0];
        for (size_t i = 0; i < dimension; ++i) {
            for (size_t j = i + 1; j < dimension; ++j) {
                NPATK::Symbol upper{static_cast<NPATK::symbol_name_t>(data[i][j])};
                NPATK::Symbol lower{static_cast<NPATK::symbol_name_t>(data[j][i])};

                if (upper != lower) {
                    output.emplace_back(upper, lower);
                }
            }
        }

        return output;
    }

    /**
     * Read through matlab sparse matrix, and identify pairs of elements that are not symmetric.
     * @param data The data array
     * @return A vector of non-matching elements, in canonical form.
     */
    std::vector<SymbolPair> identify_nonsymmetric_elements_sparse(const matlab::data::SparseArray<double>& data) {
        std::vector<SymbolPair> output{};

        // Would like to avoid this hacky copy, but random access to matlab::data::SparseArray seems not to work...
        std::map<std::pair<size_t, size_t>, NPATK::symbol_name_t> sparse_array_copy{};
        for (auto iter = data.cbegin(); iter != data.cend(); ++iter) {
            auto indices = data.getIndex(iter);
            auto value = static_cast<NPATK::symbol_name_t>(*iter);
            if (value != 0) {
                sparse_array_copy.insert(
                        sparse_array_copy.end(),
                        std::pair<std::pair<size_t, size_t>, NPATK::symbol_name_t>{indices, value}
                );
            }
        }

        // Look for unmatching elements in sparse matrix.
        for (const auto& [indices, value] : sparse_array_copy) {
            std::pair<size_t, size_t> transpose_indices{indices.second, indices.first};

            auto transposed_elem_iter = sparse_array_copy.find(transpose_indices);

            // If opposite index doesn't exist, this is a non-trivial constraint
            if (transposed_elem_iter == sparse_array_copy.end()) {
                output.emplace_back(NPATK::Symbol{value}, NPATK::Symbol{0});
                continue;
            }

            // Now, only do comparison and insert if in upper triangle
            if (indices.first > indices.second) {
                auto second_value = transposed_elem_iter->second;
                if (value != second_value) {
                    output.emplace_back(NPATK::Symbol{value}, NPATK::Symbol{second_value});
                }
            }
        }

        return output;
    }

    /**
     * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
     * @tparam datatype The data array type
     * @param data The data array
     * @return A vector of non-matching elements, in canonical form.
     */
    std::vector<SymbolPair> identify_nonsymmetric_elements(matlab::engine::MATLABEngine& engine,
                                                           const matlab::data::Array& data) {
        switch(data.getType()) {
            case matlab::data::ArrayType::SINGLE:
                return NPATK::mex::identify_nonsymmetric_elements_dense<float>(data);
            case matlab::data::ArrayType::DOUBLE:
                return NPATK::mex::identify_nonsymmetric_elements_dense<double>(data);
            case matlab::data::ArrayType::INT8:
                return NPATK::mex::identify_nonsymmetric_elements_dense<int8_t>(data);
            case matlab::data::ArrayType::UINT8:
                return NPATK::mex::identify_nonsymmetric_elements_dense<uint8_t>(data);
            case matlab::data::ArrayType::INT16:
                return NPATK::mex::identify_nonsymmetric_elements_dense<int16_t>(data);
            case matlab::data::ArrayType::UINT16:
                return NPATK::mex::identify_nonsymmetric_elements_dense<uint16_t>(data);
            case matlab::data::ArrayType::INT32:
                return NPATK::mex::identify_nonsymmetric_elements_dense<int32_t>(data);
            case matlab::data::ArrayType::UINT32:
                return NPATK::mex::identify_nonsymmetric_elements_dense<uint32_t>(data);
            case matlab::data::ArrayType::INT64:
                return NPATK::mex::identify_nonsymmetric_elements_dense<int64_t>(data);
            case matlab::data::ArrayType::UINT64:
                return NPATK::mex::identify_nonsymmetric_elements_dense<uint64_t>(data);
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                return NPATK::mex::identify_nonsymmetric_elements_sparse(data);
            default:
                break;
        }

        NPATK::mex::throw_error(engine, "Matrix type not supported (should be matrix of real numbers).");
        return std::vector<SymbolPair>{};
    }
}

class MexFunction : public matlab::mex::Function {
private:
    std::shared_ptr<matlab::engine::MATLABEngine> matlabPtr;

public:

    void operator()(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        this->matlabPtr = getEngine();

        checkArguments(outputs, inputs);
        bool isSparse = inputs[0].getType() == matlab::data::ArrayType::SPARSE_DOUBLE;
        size_t matrix_dimension = inputs[0].getDimensions()[0];

        auto raw_constraints = NPATK::mex::identify_nonsymmetric_elements(*matlabPtr, inputs[0]);

        NPATK::mex::debug_message(*matlabPtr, "Raw constraints:\n");
        std::stringstream ss;
        for (const auto& c : raw_constraints) {
            ss << c << "\n";
        }
        NPATK::mex::debug_message(*matlabPtr, ss.str());

        auto unique_constraints = NPATK::SymbolSet{raw_constraints};

        std::stringstream ss2;
        ss2 << "\nFound " << unique_constraints.symbol_count() << " symbols and "
            << unique_constraints.link_count() << " links."
            << "\nSorted, unique constraints:\n"
            << unique_constraints;
        NPATK::mex::debug_message(*matlabPtr, ss2.str());

        unique_constraints.pack();

        auto symbol_tree = NPATK::SymbolTree{unique_constraints};
        std::stringstream ss3;
        ss3 << "\nTree:\n" << symbol_tree;
        NPATK::mex::debug_message(*matlabPtr, ss3.str());

        symbol_tree.simplify();

        std::stringstream ss4;
        ss4 << "\nTree, simplified:\n" << symbol_tree;
        NPATK::mex::debug_message(*matlabPtr, ss4.str());


        unique_constraints.unpack();


        if (outputs.size() >= 1) {
            matlab::data::ArrayFactory factory;
            if (isSparse) {
                size_t nnz = 1;
                std::vector<double> data = {13.37};
                std::vector<size_t> rows = {0};
                std::vector<size_t> cols = {0};
                auto data_p = factory.createBuffer<double>(nnz);
                auto rows_p = factory.createBuffer<size_t>(nnz);
                auto cols_p = factory.createBuffer<size_t>(nnz);

                double* dataPtr = data_p.get();
                size_t* rowsPtr = rows_p.get();
                size_t* colsPtr = cols_p.get();
                std::for_each(data.begin(), data.end(), [&](const double& e) { *(dataPtr++) = e; });
                std::for_each(rows.begin(), rows.end(), [&](const size_t& e) { *(rowsPtr++) = e; });
                std::for_each(cols.begin(), cols.end(), [&](const size_t& e) { *(colsPtr++) = e; });

                auto output_array = factory.createSparseArray(
                        matlab::data::ArrayDimensions(1,1), nnz,
                            std::move(data_p), std::move(rows_p), std::move(cols_p)
                );
                outputs[0] = std::move(output_array);
            } else {
                outputs[0] = std::move(inputs[0]);
            }
        }

        this->matlabPtr.reset();
    }

    void checkArguments(matlab::mex::ArgumentList outputs, matlab::mex::ArgumentList inputs) {
        matlab::data::ArrayFactory factory;

        if (inputs.size() != 1) {
            NPATK::mex::throw_error(*this->matlabPtr, "One input required.");
        }

        auto inputDims = inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            NPATK::mex::throw_error(*this->matlabPtr, "Input must be a matrix.");
        }

        if (inputDims[0] != inputDims[1]) {
            NPATK::mex::throw_error(*this->matlabPtr, "Input must be a square matrix.");
        }
    }
};