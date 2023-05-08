/**
 * read_eigen_dense.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_eigen_dense.h"

#include "error_codes.h"
#include "utilities/reporting.h"
#include "utilities/visitor.h"
#include "utilities/read_as_scalar.h"

#include "utilities/utf_conversion.h"

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include <Eigen/Dense>

namespace Moment::mex {

    namespace {
        struct ReadDenseNumericMatrixVisitor {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = Eigen::MatrixXd;

            explicit ReadDenseNumericMatrixVisitor(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<double> data_t>
            return_type dense(const matlab::data::TypedArray<data_t>& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                Eigen::MatrixXd output(dims[0], dims[1]);

                for (int row = 0; row < dims[0]; ++row) {
                    for (int col = 0; col < dims[1]; ++col) {
                        output(row,col) = input_matrix[row][col];
                    }
                }

                return output;
            }

            template<std::convertible_to<double> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t>& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                Eigen::MatrixXd output(dims[0], dims[1]);
                output.setZero();

                for (matlab::data::TypedIterator<const data_t> iter = input_matrix.begin();
                     iter != input_matrix.end(); ++iter) {
                    auto indices = input_matrix.getIndex(iter);
                    output(indices.first, indices.second) = *iter;
                }

                return output;
            }


            return_type string(const matlab::data::StringArray& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                Eigen::MatrixXd output(dims[0], dims[1]);

                UTF16toUTF8Convertor convertor;

                for (int row = 0; row < dims[0]; ++row) {
                    for (int col = 0; col < dims[1]; ++col) {
                        const auto& elem = input_matrix[row][col];

                        // Treat empty string as 0.
                        if (!elem.has_value()) {
                            output(row,col) = 0;
                            continue;
                        }

                        try {
                            std::string utf8str = static_cast<std::string>(elem);
                            std::stringstream ss{utf8str};
                            double value;
                            ss >> value;
                            output(row,col) = value;
                        } catch (std::exception& e) {
                            std::stringstream errSS;
                            errSS << "Could not parse string at index (" << (row+1) << ", " << (col+1) << ") as number.";
                            throw errors::unreadable_scalar{errors::could_not_convert, errSS.str()};
                        }
                    }
                }

                return output;
            }

        };

        static_assert(concepts::VisitorHasRealDense<ReadDenseNumericMatrixVisitor>);
        static_assert(concepts::VisitorHasRealSparse<ReadDenseNumericMatrixVisitor>);
        static_assert(concepts::VisitorHasString<ReadDenseNumericMatrixVisitor>);
    }
    /** Reads a matlab matrix into a real eigen sparse matrix */
    Eigen::MatrixXd read_eigen_dense(matlab::engine::MATLABEngine& engine,
                                     const matlab::data::Array& input) {
        return DispatchVisitor(engine, input, ReadDenseNumericMatrixVisitor{engine});
    }

}