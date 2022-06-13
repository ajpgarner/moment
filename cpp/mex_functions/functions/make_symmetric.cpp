/**
 * make_symmetric.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "make_symmetric.h"

#include "mex.hpp"

#include <vector>
#include <map>
#include <sstream>
#include <algorithm>

#include "symbolic/symbol.h"
#include "symbolic/symbol_set.h"
#include "symbolic/symbol_tree.h"

#include "fragments/export_substitution_list.h"
#include "fragments/substitute_elements_using_tree.h"
#include "fragments/read_symbol_or_fail.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"


namespace NPATK::mex::functions {

    namespace {
        /**
         * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
         */
        class NonsymmetricElementIdentifierVisitor {
            matlab::engine::MATLABEngine& engine;

        public:
            using return_type = NPATK::SymbolSet;

        public:
            explicit NonsymmetricElementIdentifierVisitor(matlab::engine::MATLABEngine &the_engine)
                : engine(the_engine) { }

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @tparam datatype The data array type
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                SymbolSet output{};

                std::vector<SymbolPair> non_matching{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    // Register diagonal element as real symbol:
                    NPATK::SymbolExpression diag{static_cast<NPATK::symbol_name_t>(data[i][i])};
                    output.add_or_merge(Symbol{diag.id, false});

                    for (size_t j = i + 1; j < dimension; ++j) {
                        NPATK::SymbolExpression upper{static_cast<NPATK::symbol_name_t>(data[i][j])};
                        NPATK::SymbolExpression lower{static_cast<NPATK::symbol_name_t>(data[j][i])};

                        if (upper != lower) {
                            output.add_or_merge(SymbolPair{upper, lower}, true);
                        } else {
                            output.add_or_merge(Symbol{upper.id, false});
                            output.add_or_merge(Symbol{lower.id, false});
                        }
                    }
                }

                return output;
            }

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            return_type string(const matlab::data::StringArray &data) {
                SymbolSet output{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    // Register diagonal element as real symbol:
                    NPATK::SymbolExpression diag{read_symbol_or_fail(this->engine, data, i, i)};
                    output.add_or_merge(Symbol{diag.id, false});

                    for (size_t j = i + 1; j < dimension; ++j) {
                        NPATK::SymbolExpression upper{read_symbol_or_fail(this->engine, data, i, j)};
                        NPATK::SymbolExpression lower{read_symbol_or_fail(this->engine, data, j, i)};

                        if (upper != lower) {
                            output.add_or_merge(SymbolPair{upper, lower});
                        } else {
                            output.add_or_merge(Symbol{upper.id, false});
                            output.add_or_merge(Symbol{lower.id, false});
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
            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type sparse(const matlab::data::SparseArray<datatype>& data) {

                // Would like to avoid this hacky input copy,
                //   but random access to matlab::data::SparseArray seems not to work...
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
                SymbolSet output{};
                for (const auto &[indices, value]: sparse_array_copy) {
                    // Add diagonal element
                    if (indices.first == indices.second) {
                        output.add_or_merge(Symbol{value, false});
                        continue;
                    }

                    std::pair<size_t, size_t> transpose_indices{indices.second, indices.first};

                    auto transposed_elem_iter = sparse_array_copy.find(transpose_indices);

                    // If opposite index doesn't exist, this is a non-trivial constraint
                    if (transposed_elem_iter == sparse_array_copy.end()) {
                        output.add_or_merge(SymbolPair{NPATK::SymbolExpression{value},
                                                       NPATK::SymbolExpression{0}});
                        continue;
                    }

                    // Now, only do comparison and insert if in upper triangle
                    if (indices.first > indices.second) {
                        auto second_value = transposed_elem_iter->second;
                        if (value != second_value) {
                            output.add_or_merge(SymbolPair{NPATK::SymbolExpression{value},
                                                           NPATK::SymbolExpression{second_value}});
                        } else {
                            output.add_or_merge(Symbol{value, false});
                            output.add_or_merge(Symbol{second_value, false});
                        }
                    }
                }

                return output;
            }


        };

        static_assert(concepts::VisitorHasRealDense<NonsymmetricElementIdentifierVisitor>);
        static_assert(concepts::VisitorHasRealSparse<NonsymmetricElementIdentifierVisitor>);
        static_assert(concepts::VisitorHasString<NonsymmetricElementIdentifierVisitor>);

        /**
         * Read through matlab dense numerical matrix, and identify pairs of elements that are not symmetric.
         * @param engine Reference to matlab engine
         * @param data The data array
         * @return A SymbolSet of elements in the matrix, with raw inferred equalities.
         */
        SymbolSet identify_nonsymmetric_elements(matlab::engine::MATLABEngine &engine,
                                                               const matlab::data::Array &data) {
            return DispatchVisitor(engine, data, NonsymmetricElementIdentifierVisitor{engine});
        }
    }


    MakeSymmetric::MakeSymmetric(matlab::engine::MATLABEngine &matlabEngine)
            : MexFunction(matlabEngine, MEXEntryPointID::MakeSymmetric, u"make_symmetric") {
        this->flag_names.emplace(u"dense");
        this->flag_names.emplace(u"sparse");
        this->mutex_params.add_mutex(u"dense", u"sparse");

        this->min_outputs = 1;
        this->max_outputs = 2;
        this->min_inputs = 1;
        this->max_inputs = 1;
    }

    std::unique_ptr<SortedInputs> MakeSymmetric::transform_inputs(std::unique_ptr<SortedInputs> inputPtr) const {
        auto& input = *inputPtr;
        assert(!input.inputs.empty());

        auto inputDims = input.inputs[0].getDimensions();
        if (inputDims.size() != 2) {
            throw errors::BadInput{errors::bad_param, "Input must be a matrix."};
        }

        if (inputDims[0] != inputDims[1]) {
            throw errors::BadInput{errors::bad_param, "Input must be a square matrix."};
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
            case matlab::data::ArrayType::MATLAB_STRING:
                break;
            default:
                throw errors::BadInput{errors::bad_param, "Matrix type must be real numeric, or of strings."};
        }

        return std::make_unique<MakeSymmetricParams>(std::move(input));
    }


    MakeSymmetricParams::MakeSymmetricParams(SortedInputs &&structuredInputs)
        : SortedInputs(std::move(structuredInputs)) {
        // Determine sparsity of output
        this->sparse_output = (inputs[0].getType() == matlab::data::ArrayType::SPARSE_DOUBLE);
        if (flags.contains(u"sparse")) {
            this->sparse_output = true;
        } else if (flags.contains(u"dense")) {
            this->sparse_output = false;
        }
    }


    void MakeSymmetric::operator()(IOArgumentRange outputs, std::unique_ptr<SortedInputs> inputPtr) {
        auto& inputs = dynamic_cast<MakeSymmetricParams&>(*inputPtr);

        auto unique_constraints = identify_nonsymmetric_elements(matlabEngine, inputs.inputs[0]);

        if (verbose) {
            std::stringstream ss2;
            ss2 << "\nFound " << unique_constraints.symbol_count() << " symbols and "
                << unique_constraints.link_count() << " links.\n";
            if (debug) {
                ss2 << "Sorted, unique constraints:\n"
                    << unique_constraints;
            }
            NPATK::mex::print_to_console(matlabEngine, ss2.str());
        }

        unique_constraints.pack();
        auto symbol_tree = NPATK::SymbolTree{std::move(unique_constraints)};

        if (debug) {
            std::stringstream ss3;
            ss3 << "\nTree, initial:\n" << symbol_tree;
            NPATK::mex::print_to_console(matlabEngine, ss3.str());
        }

        symbol_tree.simplify();

        if (verbose) {
            std::stringstream ss4;
            ss4 << "\nTree, simplified:\n" << symbol_tree << "\n";
            NPATK::mex::print_to_console(matlabEngine, ss4.str());
        }


        if (outputs.size() >= 1) {
            outputs[0] = NPATK::mex::make_symmetric_using_tree(matlabEngine, inputs.inputs[0],
                                                               symbol_tree, inputs.sparse_output);
        }

        if (outputs.size() >= 2) {
            outputs[1] = NPATK::mex::export_substitution_list(matlabEngine, symbol_tree);
        }
    }

}