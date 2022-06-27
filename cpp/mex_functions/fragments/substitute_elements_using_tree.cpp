/**
 * make_symmetric_using_tree.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "../utilities/make_sparse_matrix.h"
#include "../utilities/reporting.h"
#include "../utilities/visitor.h"

#include "substitute_elements_using_tree.h"
#include "read_symbol_or_fail.h"

namespace NPATK::mex {
    namespace {
        class MakeDenseSymMatrixVisitor {
        private:
            matlab::engine::MATLABEngine& engine;
            const SymbolTree& tree;
        public:
            using return_type = matlab::data::TypedArray<int64_t>;

            explicit MakeDenseSymMatrixVisitor(matlab::engine::MATLABEngine& the_engine, const SymbolTree& the_tree)
                : engine{the_engine}, tree{the_tree} { }

            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &input) {
                matlab::data::ArrayFactory factory;

                const auto matrix_dims = input.getDimensions();
                return_type output = factory.createArray<int64_t>(matrix_dims);
                auto out_iter = output.begin();

                for (auto & elem : input) {
                    assert(out_iter != output.end());
                    NPATK::SymbolExpression existing_symbol{static_cast<NPATK::symbol_name_t>(elem)};                     
                    NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);

                    *out_iter = new_symbol.as_integer();
                    ++out_iter;
                }
                assert(out_iter == output.end());

                return output;
            }

            return_type string(const matlab::data::StringArray &input) {
                matlab::data::ArrayFactory factory;

                const auto matrix_dims = input.getDimensions();
                return_type output = factory.createArray<int64_t>(matrix_dims);

                for (size_t row = 0; row < matrix_dims[0]; ++row) {
                    for (size_t col = 0; col < matrix_dims[1]; ++col) {
                        NPATK::SymbolExpression existing_symbol{read_symbol_or_fail(this->engine, input, row, col)};
                        NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);

                        output[row][col] = new_symbol.as_integer();
                    }
                }
                return output;
            }

            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type sparse(const matlab::data::SparseArray<datatype> &input) {
                matlab::data::ArrayFactory factory;
                const auto matrix_dims = input.getDimensions();
                matlab::data::TypedArray<int64_t> output = factory.createArray<int64_t>(matrix_dims);

                auto input_iter = input.cbegin();
                while (input_iter != input.cend()) {
                    NPATK::SymbolExpression existing_symbol{static_cast<NPATK::symbol_name_t>(*input_iter)};
                    NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
                    int64_t ns_symbol_int = new_symbol.as_integer();

                    auto [row, col] = input.getIndex(input_iter);

                    output[row][col] = ns_symbol_int;
                    output[col][row] = ns_symbol_int;

                    ++input_iter;
                }
                return output;
            }
        };

        static_assert(concepts::VisitorHasRealDense<MakeDenseSymMatrixVisitor>);
        static_assert(concepts::VisitorHasRealSparse<MakeDenseSymMatrixVisitor>);
        static_assert(concepts::VisitorHasString<MakeDenseSymMatrixVisitor>);

        class MakeSparseSymMatrixVisitor {
        private:
            matlab::engine::MATLABEngine& engine;
            const SymbolTree& tree;
        public:
            using return_type = matlab::data::SparseArray<double>;

            explicit MakeSparseSymMatrixVisitor(matlab::engine::MATLABEngine& the_engine, const SymbolTree& the_tree)
                : engine(the_engine), tree{the_tree} { }

            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type dense(const matlab::data::TypedArray<datatype> &input) {
                matlab::data::ArrayFactory factory;

                const auto matrix_dims = input.getDimensions();
                sparse_set_build<double> output_build{};

                for (size_t row = 0; row < matrix_dims[0]; ++row) {
                    for (size_t col = 0; col < matrix_dims[1]; ++col) {
                        NPATK::SymbolExpression existing_symbol{static_cast<NPATK::symbol_name_t>(input[row][col])};
                        NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
                        output_build.emplace_hint(output_build.end(), std::make_pair(row,col), new_symbol.as_integer());;
                    }
                }

                return make_sparse_matrix(this->engine, {matrix_dims[0], matrix_dims[1]}, output_build);
            }

            return_type string(const matlab::data::StringArray &input) {
                matlab::data::ArrayFactory factory;

                const auto matrix_dims = input.getDimensions();
                sparse_set_build<double> output_build{};

                for (size_t row = 0; row < matrix_dims[0]; ++row) {
                    for (size_t col = 0; col < matrix_dims[1]; ++col) {
                        NPATK::SymbolExpression existing_symbol{read_symbol_or_fail(this->engine, input, row, col)};
                        NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
                        output_build.emplace_hint(output_build.end(), std::make_pair(row,col), new_symbol.as_integer());;
                    }
                }

                return make_sparse_matrix(this->engine, {matrix_dims[0], matrix_dims[1]}, output_build);
            }

            template<std::convertible_to<NPATK::symbol_name_t> datatype>
            return_type sparse(const matlab::data::SparseArray<datatype> &input) {
                matlab::data::ArrayFactory factory;

                const auto matrix_dims = input.getDimensions();
                sparse_set_build<double> output_build{};

                auto input_iter = input.cbegin();
                while (input_iter != input.cend()) {
                    NPATK::SymbolExpression existing_symbol{static_cast<NPATK::symbol_name_t>(*input_iter)};
                    NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
                    int64_t ns_symbol_int = new_symbol.as_integer();

                    auto [row, col] = input.getIndex(input_iter);

                    output_build.emplace_hint(output_build.end(), std::make_pair(row,col), ns_symbol_int);
                    output_build.emplace(std::make_pair(col, row), ns_symbol_int);

                    ++input_iter;
                }
                return make_sparse_matrix(this->engine, {matrix_dims[0], matrix_dims[1]}, output_build);
            }


        };

        static_assert(concepts::VisitorHasRealDense<MakeSparseSymMatrixVisitor>);
        static_assert(concepts::VisitorHasRealSparse<MakeSparseSymMatrixVisitor>);
        static_assert(concepts::VisitorHasString<MakeSparseSymMatrixVisitor>);


    }

    matlab::data::Array make_symmetric_using_tree(matlab::engine::MATLABEngine& engine,
                                                  const matlab::data::Array& the_array,
                                                  const SymbolTree& tree, bool sparse_output) {
        if (sparse_output) {
            return DispatchVisitor(engine, the_array, MakeSparseSymMatrixVisitor{engine, tree});
        }

        return DispatchVisitor(engine, the_array, MakeDenseSymMatrixVisitor{engine, tree});
    }

    matlab::data::TypedArray<matlab::data::MATLABString> make_hermitian_using_tree(
            matlab::engine::MATLABEngine &engine,
            const matlab::data::TypedArray<matlab::data::MATLABString> &input,
            const SymbolTree &tree) {

        matlab::data::ArrayFactory factory;

        const auto matrix_dims = input.getDimensions();
        auto output = factory.createArray<matlab::data::MATLABString>(matrix_dims);

        for (size_t row = 0; row < matrix_dims[0]; ++row) {
            for (size_t col = 0; col < matrix_dims[1]; ++col) {
                NPATK::SymbolExpression existing_symbol{read_symbol_or_fail(engine, input, row, col)};
                NPATK::SymbolExpression new_symbol = tree.substitute(existing_symbol);
                output[row][col] = new_symbol.as_string();
            }
        }
        return output;
    }
}
