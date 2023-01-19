/**
 * read_raw_symbol_matrix.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "read_raw_symbol_matrix.h"

#include "utilities/visitor.h"

#include "mex.hpp"


namespace Moment::mex {
    namespace {
        struct ReadSymbolicMatrixVisitor {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = std::unique_ptr<SquareMatrix<SymbolExpression>>;

            explicit ReadSymbolicMatrixVisitor(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &input_matrix) {
                size_t matrix_dimension = input_matrix.getDimensions()[0];
                std::vector<SymbolExpression> data;
                data.reserve(matrix_dimension * matrix_dimension);

                // Read through matrix, into vector
                for (auto x: input_matrix) {
                    auto sym_id = static_cast<symbol_name_t>(x);
                    if (sym_id >= 0) {
                        data.emplace_back(sym_id);
                    } else {
                        data.emplace_back(-sym_id, -1.0);
                    }
                }
                return std::make_unique<SquareMatrix<SymbolExpression>>(matrix_dimension, std::move(data));
            }
        };
    }

    std::unique_ptr<SquareMatrix<SymbolExpression>>
    read_raw_symbol_matrix(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array& input) {
        return DispatchVisitor(matlabEngine, input, ReadSymbolicMatrixVisitor{matlabEngine});
    }
}