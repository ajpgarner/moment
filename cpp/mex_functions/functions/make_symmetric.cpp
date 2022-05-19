/*
 * (c) 2022-2022 Austrian Academy of Sciences.
 *
 * NPAToolKit is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
#include "make_symmetric.h"

#include "mex.hpp"

#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include "symbol.h"
#include "symbol_set.h"
#include "symbol_tree.h"

#include "../helpers/reporting.h"
#include "../helpers/substitute_elements_using_tree.h"
#include "../helpers/export_substitution_list.h"



namespace NPATK::mex::functions {
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
                NPATK::SymbolExpression upper{static_cast<NPATK::symbol_name_t>(data[i][j])};
                NPATK::SymbolExpression lower{static_cast<NPATK::symbol_name_t>(data[j][i])};

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
                output.emplace_back(NPATK::SymbolExpression{value}, NPATK::SymbolExpression{0});
                continue;
            }

            // Now, only do comparison and insert if in upper triangle
            if (indices.first > indices.second) {
                auto second_value = transposed_elem_iter->second;
                if (value != second_value) {
                    output.emplace_back(NPATK::SymbolExpression{value}, NPATK::SymbolExpression{second_value});
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
                return identify_nonsymmetric_elements_dense<float>(data);
            case matlab::data::ArrayType::DOUBLE:
                return identify_nonsymmetric_elements_dense<double>(data);
            case matlab::data::ArrayType::INT8:
                return identify_nonsymmetric_elements_dense<int8_t>(data);
            case matlab::data::ArrayType::UINT8:
                return identify_nonsymmetric_elements_dense<uint8_t>(data);
            case matlab::data::ArrayType::INT16:
                return identify_nonsymmetric_elements_dense<int16_t>(data);
            case matlab::data::ArrayType::UINT16:
                return identify_nonsymmetric_elements_dense<uint16_t>(data);
            case matlab::data::ArrayType::INT32:
                return identify_nonsymmetric_elements_dense<int32_t>(data);
            case matlab::data::ArrayType::UINT32:
                return identify_nonsymmetric_elements_dense<uint32_t>(data);
            case matlab::data::ArrayType::INT64:
                return identify_nonsymmetric_elements_dense<int64_t>(data);
            case matlab::data::ArrayType::UINT64:
                return identify_nonsymmetric_elements_dense<uint64_t>(data);
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                return identify_nonsymmetric_elements_sparse(data);
            default:
                break;
        }

        NPATK::mex::throw_error(engine, "Matrix type not supported (should be matrix of real numbers).");
        return std::vector<SymbolPair>{};
    }


    MakeSymmetric::MakeSymmetric(matlab::engine::MATLABEngine &matlabEngine)
            : MexFunction(matlabEngine, MEXEntryPointID::MakeSymmetric, u"make_symmetric") {
        this->min_outputs = 1;
        this->max_outputs = 2;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::pair<bool, std::basic_string<char16_t>> MakeSymmetric::validate_inputs(const SortedInputs &input) const {
        // Should be guaranteed~
        assert(!input.inputs.empty());

        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            return {false, u"Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            return {false, u"Input must be a square matrix."};
        }

        switch(input.inputs[0].getType()) {
            case matlab::data::ArrayType::SINGLE:
            case matlab::data::ArrayType::DOUBLE:
            case matlab::data::ArrayType::INT8:
            case matlab::data::ArrayType::UINT8:
            case matlab::data::ArrayType::INT16:
            case matlab::data::ArrayType::UINT16:
            case matlab::data::ArrayType::INT32:
            case matlab::data::ArrayType::UINT32:
            case matlab::data::ArrayType::INT64:
            case matlab::data::ArrayType::UINT64:
            case matlab::data::ArrayType::SPARSE_DOUBLE:
                break;
            default:
                return {false, u"Matrix type must be numeric."};
        }

        return {true, u""};
    }


    void MakeSymmetric::operator()(FlagArgumentRange outputs, SortedInputs&& inputs) {
        // If no output, nothing to do...
        if (outputs.size() <= 0) {
            return;
        }

        bool debug = (inputs.flags.contains(u"debug"));
        bool verbose = debug || (inputs.flags.contains(u"verbose"));

        auto raw_constraints = identify_nonsymmetric_elements(matlabEngine, inputs.inputs[0]);

        if (debug) {
            NPATK::mex::print_to_console(matlabEngine, "Raw constraints:\n");
            std::stringstream ss;
            for (const auto &c: raw_constraints) {
                ss << c << "\n";
            }
            NPATK::mex::print_to_console(matlabEngine, ss.str());
        }

        auto unique_constraints = NPATK::SymbolSet{raw_constraints};

        if (verbose) {
            std::stringstream ss2;
            ss2 << "\nFound " << unique_constraints.symbol_count() << " symbols and "
                << unique_constraints.link_count() << " links.\n";
            ss2 << "Sorted, unique constraints:\n"
                << unique_constraints;
            NPATK::mex::print_to_console(matlabEngine, ss2.str());
        }

        unique_constraints.pack();
        auto symbol_tree = NPATK::SymbolTree{std::move(unique_constraints)};

        if (verbose) {
            std::stringstream ss3;
            ss3 << "\nTree, initial:\n" << symbol_tree;
            NPATK::mex::print_to_console(matlabEngine, ss3.str());
        }

        symbol_tree.simplify();

        if (verbose) {
            std::stringstream ss4;
            ss4 << "\nTree, simplified:\n" << symbol_tree;
            NPATK::mex::print_to_console(matlabEngine, ss4.str());
        }

        if (outputs.size() >= 1) {
            outputs[0] = NPATK::mex::substitute_elements_using_tree(matlabEngine, std::move(inputs.inputs[0]), symbol_tree);
        }

        if (outputs.size() >= 2) {
            outputs[1] = NPATK::mex::export_substitution_list(matlabEngine, symbol_tree);
        }

           /* matlab::data::ArrayFactory factory;
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
            }*/
    }
}