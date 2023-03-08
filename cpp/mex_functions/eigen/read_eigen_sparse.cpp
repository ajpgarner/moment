/**
 * read_eigen_sparse.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "read_eigen_sparse.h"

#include "utilities/visitor.h"
#include "utilities/read_as_scalar.h"

#include "mex.hpp"
#include "MatlabDataArray.hpp"

namespace Moment::mex {

    namespace {
        struct ReadNumericMatrixVisitor {
        private:
            matlab::engine::MATLABEngine &engine;

        public:
            using return_type = Eigen::SparseMatrix<double>;

            explicit ReadNumericMatrixVisitor(matlab::engine::MATLABEngine &engineRef)
                    : engine(engineRef) {}

            template<std::convertible_to<double> data_t>
            return_type dense(const matlab::data::TypedArray<data_t>& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                std::vector<Eigen::Triplet<double>> triplets;

                for (int row = 0; row < dims[0]; ++row) {
                    for (int col = 0; col < dims[1]; ++col) {
                        auto val = static_cast<double>(input_matrix[row][col]);
                        if (val != 0) {
                            triplets.emplace_back(row, col, val);
                        }
                    }
                }

                return triplet_to_sparse({dims[0], dims[1]}, triplets);
            }

            template<std::convertible_to<double> data_t>
            return_type sparse(const matlab::data::SparseArray<data_t>& input_matrix) {
                const auto dims = input_matrix.getDimensions();
                std::vector<Eigen::Triplet<double>> triplets;
                triplets.reserve(input_matrix.getNumberOfNonZeroElements());

                for (matlab::data::TypedIterator<const data_t> iter = input_matrix.begin();
                     iter != input_matrix.end(); ++iter) {
                    auto indices = input_matrix.getIndex(iter);
                    triplets.emplace_back(static_cast<int>(indices.first),
                                          static_cast<int>(indices.second),
                                          static_cast<double>(*iter));
                }

                return triplet_to_sparse({dims[0], dims[1]}, triplets);
            }


            return_type string(const matlab::data::StringArray& input_matrix) {
                const auto dims = input_matrix.getDimensions();

                std::vector<Eigen::Triplet<double>> triplets;

                for (int row = 0; row < dims[0]; ++row) {
                    for (int col = 0; col < dims[1]; ++col) {
                        const auto& elem = input_matrix[row][col];

                        // Treat empty string as 0.
                        if (!elem.has_value()) {
                            continue;
                        }

                        try {
                            std::string utf8str = matlab::engine::convertUTF16StringToUTF8String(elem);
                            std::stringstream ss{utf8str};
                            double value;
                            ss >> value;
                            if (value != 0) {
                                triplets.emplace_back(row, col, value);
                            }
                        } catch (std::exception& e) {
                            std::stringstream errSS;
                            errSS << "Could not parse string at index (" << (row+1) << ", " << (col+1) << ") as number.";
                            throw errors::unreadable_scalar{errors::could_not_convert, errSS.str()};
                        }
                    }
                }

                return triplet_to_sparse({dims[0], dims[1]}, triplets);
            }

        private:
            static return_type triplet_to_sparse(std::pair<size_t,size_t> dims,
                                                 const std::vector<Eigen::Triplet<double>>& triplet) {
                return_type matrix(static_cast<int>(dims.first), static_cast<int>(dims.second));
                if (!triplet.empty()) {
                    matrix.setFromTriplets(triplet.begin(), triplet.end());
                } else {
                    matrix.setZero();
                }
                return matrix;
            }
        };

        static_assert(concepts::VisitorHasRealDense<ReadNumericMatrixVisitor>);
        static_assert(concepts::VisitorHasRealSparse<ReadNumericMatrixVisitor>);
        static_assert(concepts::VisitorHasString<ReadNumericMatrixVisitor>);
    }

    Eigen::SparseMatrix<double> read_eigen_sparse(matlab::engine::MATLABEngine& engine,
                                                  const matlab::data::Array& input) {

        return DispatchVisitor(engine, input, ReadNumericMatrixVisitor{engine});

    }
}