/**
 * identify_nonsymmetric_elements.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "identify_nonsymmetric_elements.h"

#include "fragments/read_symbol_or_fail.h"
#include "utilities/make_sparse_matrix.h"
#include "utilities/visitor.h"

#include "MatlabDataArray.hpp"

namespace Moment::mex {

    namespace {
        /**
          * Read through matlab matrix, and identify if not symmetric anywhere
          */
        class IsSymmetricVisitor {
            matlab::engine::MATLABEngine& engine;

        public:
            using return_type = bool;

        public:
            explicit IsSymmetricVisitor(matlab::engine::MATLABEngine &the_engine)
                    : engine(the_engine) { }

            /**
              * Read through matlab dense numerical matrix, and identify pairs of elements that are never symmetric.
              * @tparam datatype The data array type
              * @param data The data array
              * @return A vector of non-matching elements, in canonical form.
              */
            template<std::convertible_to<symbol_name_t> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &data) {
                SymbolSet output{};

                const size_t dimension = data.getDimensions()[0];
                for (size_t i = 0; i < dimension; ++i) {
                    for (size_t j = i + 1; j < dimension; ++j) {
                        SymbolExpression upper{static_cast<symbol_name_t>(data[i][j])};
                        SymbolExpression lower{static_cast<symbol_name_t>(data[j][i])};

                        if (upper != lower) {
                            return false;
                        }
                    }
                }

                return true;
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
                    for (size_t j = i + 1; j < dimension; ++j) {
                        SymbolExpression upper{read_symbol_or_fail(this->engine, data, i, j)};
                        SymbolExpression lower{read_symbol_or_fail(this->engine, data, j, i)};

                        if (upper != lower) {
                            return false;
                        }
                    }
                }
                return true;
            }


            /**
             * Read through matlab sparse matrix, and identify pairs of elements that are cannot be symmetric.
             * @param data The data array
             * @return A vector of non-matching elements, in canonical form.
             */
            template<std::convertible_to<symbol_name_t> datatype>
            return_type sparse(const matlab::data::SparseArray<datatype>& data) {

                // Get sparse array as something that can be randomly accessed...
                auto sparse_array_copy = sparse_array_to_map<datatype, symbol_name_t>(data);

                // Look for non-matching elements in sparse matrix.
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
                        return false;
                    }

                    // Now, only do comparison if in upper triangle
                    if (indices.first > indices.second) {
                        auto second_value = transposed_elem_iter->second;
                        if (value != second_value) {
                            return false;
                        }
                    }
                }

                // No mismatch found, is symmetric...
                return true;
            }


        };

        static_assert(concepts::VisitorHasRealDense<IsSymmetricVisitor>);
        static_assert(concepts::VisitorHasRealSparse<IsSymmetricVisitor>);
        static_assert(concepts::VisitorHasString<IsSymmetricVisitor>);
    }

    bool is_symmetric(matlab::engine::MATLABEngine &engine, const matlab::data::Array &data) {
        return DispatchVisitor(engine, data, IsSymmetricVisitor{engine});
    }
}
