/**
 * read_raw_symbol_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_raw_symbol_matrix.h"

#include "read_symbol_or_fail.h"
#include "utilities/visitor.h"

#include "mex.hpp"


namespace Moment::mex {
    namespace {
        struct ReadSymbolicMatrixVisitor {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = std::unique_ptr<SquareMatrix<Monomial>>;

            explicit ReadSymbolicMatrixVisitor(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<symbol_name_t> data_t>
            return_type dense(const matlab::data::TypedArray<data_t> &input_matrix) {
                const size_t matrix_dimension = input_matrix.getDimensions()[0];
                std::vector<Monomial> data;
                data.reserve(matrix_dimension * matrix_dimension);

                // Col major data -> col major data
                for (const auto& input_elem : input_matrix) {
                    auto sym_id = static_cast<symbol_name_t>(input_elem);
                    if (sym_id >= 0) {
                        data.emplace_back(sym_id);
                    } else {
                        data.emplace_back(-sym_id, -1.0);
                    }
                }

                return std::make_unique<SquareMatrix<Monomial>>(matrix_dimension, std::move(data));
            }


            return_type string(const matlab::data::StringArray &input_matrix) {
                const size_t matrix_dimension = input_matrix.getDimensions()[0];
                std::vector<Monomial> data;
                data.reserve(matrix_dimension * matrix_dimension);

                for (size_t col = 0; col < matrix_dimension; ++col) {
                    for (size_t row = 0; row < matrix_dimension; ++row) {
                        data.push_back(read_symbol_or_fail(this->engine, input_matrix, row, col));
                    }
                }

                return std::make_unique<SquareMatrix<Monomial>>(matrix_dimension, std::move(data));
            }
        };
    }

    std::unique_ptr<SquareMatrix<Monomial>>
    read_raw_symbol_matrix(matlab::engine::MATLABEngine &matlabEngine, const matlab::data::Array& input) {
        return DispatchVisitor(matlabEngine, input, ReadSymbolicMatrixVisitor{matlabEngine});
    }
}