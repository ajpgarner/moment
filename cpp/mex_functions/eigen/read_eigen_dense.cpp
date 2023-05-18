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
        template<typename scalar_type = double>
        struct ReadDenseNumericMatrixVisitor {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = Eigen::Matrix<scalar_type, -1, -1>;

            explicit ReadDenseNumericMatrixVisitor(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<scalar_type> data_t>
            return_type dense(const matlab::data::TypedArray<data_t>& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                return_type output(dims[0], dims[1]);

                for (int row = 0; row < dims[0]; ++row) {
                    for (int col = 0; col < dims[1]; ++col) {
                        output(row,col) = static_cast<scalar_type>(input_matrix[row][col]);
                    }
                }

                return output;
            }

            template<std::convertible_to<scalar_type> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t>& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                return_type output(dims[0], dims[1]);
                output.setZero();

                for (matlab::data::TypedIterator<const data_t> iter = input_matrix.begin();
                     iter != input_matrix.end(); ++iter) {
                    auto indices = input_matrix.getIndex(iter);
                    output(indices.first, indices.second) = static_cast<scalar_type>(*iter);
                }

                return output;
            }


            return_type string(const matlab::data::StringArray& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                return_type output(dims[0], dims[1]);

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
                            output(row,col) = static_cast<scalar_type>(value);
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

        static_assert(concepts::VisitorHasRealDense<ReadDenseNumericMatrixVisitor<double>>);
        static_assert(concepts::VisitorHasRealSparse<ReadDenseNumericMatrixVisitor<double>>);
        static_assert(concepts::VisitorHasString<ReadDenseNumericMatrixVisitor<double>>);
        static_assert(std::is_same_v<ReadDenseNumericMatrixVisitor<double>::return_type, Eigen::MatrixXd>);


        static_assert(concepts::VisitorHasRealDense<ReadDenseNumericMatrixVisitor<std::complex<double>>>);
        static_assert(concepts::VisitorHasRealSparse<ReadDenseNumericMatrixVisitor<std::complex<double>>>);
        static_assert(concepts::VisitorHasString<ReadDenseNumericMatrixVisitor<std::complex<double>>>);
        static_assert(concepts::VisitorHasComplexDenseFloat<ReadDenseNumericMatrixVisitor<std::complex<double>>>);
        static_assert(concepts::VisitorHasComplexSparse<ReadDenseNumericMatrixVisitor<std::complex<double>>>);
        static_assert(std::is_same_v<ReadDenseNumericMatrixVisitor<std::complex<double>>::return_type, Eigen::MatrixXcd>);
    }


    /** Reads a matlab matrix into a real eigen sparse matrix */
    Eigen::MatrixXd read_eigen_dense(matlab::engine::MATLABEngine& engine,
                                     const matlab::data::Array& input) {
        return DispatchVisitor(engine, input, ReadDenseNumericMatrixVisitor<double>{engine});
    }

    /** Reads a matlab matrix into a real eigen sparse matrix */
    Eigen::MatrixXcd read_eigen_dense_complex(matlab::engine::MATLABEngine& engine,
                                              const matlab::data::Array& input) {
        return DispatchVisitor(engine, input, ReadDenseNumericMatrixVisitor<std::complex<double>>{engine});
    }

}