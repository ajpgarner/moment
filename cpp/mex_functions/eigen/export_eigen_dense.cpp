/**
 * export_eigen_dense.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_eigen_dense.h"

#include "errors.h"
#include "utilities/reporting.h"

namespace Moment::mex {
    matlab::data::TypedArray<double> export_eigen_dense(matlab::engine::MATLABEngine& engine,
                                                        matlab::data::ArrayFactory& factory,
                                                        const Eigen::MatrixXd& matrix) {
        const matlab::data::ArrayDimensions dims{static_cast<size_t>(matrix.rows()),
                                           static_cast<size_t>(matrix.cols())};

        // MATLAB is col major; but so is eigen...
        return factory.createArray<double>(dims, matrix.data(), matrix.data() + (dims[0]*dims[1]));
    }

    matlab::data::TypedArray<std::complex<double>> export_eigen_dense(matlab::engine::MATLABEngine& engine,
                                                        matlab::data::ArrayFactory& factory,
                                                        const Eigen::MatrixXcd& matrix) {
        const matlab::data::ArrayDimensions dims{static_cast<size_t>(matrix.rows()),
                                                 static_cast<size_t>(matrix.cols())};

        // MATLAB is col major; but so is eigen...
        return factory.createArray<std::complex<double>>(dims, matrix.data(), matrix.data() + (dims[0]*dims[1]));
    }

    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_dense_array(matlab::engine::MATLABEngine& engine,
                             matlab::data::ArrayFactory& factory,
                             const std::vector<Eigen::MatrixXd>& matrices) {

        matlab::data::ArrayDimensions dims{1, matrices.size()};
        matlab::data::TypedArray<matlab::data::Array> output = factory.createCellArray(dims);
        auto write_iter = output.begin();
        for (const auto& matrix : matrices) {
            *write_iter = export_eigen_dense(engine, factory, matrix);
            ++write_iter;
        }

        return output;
    }

    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_dense_array(matlab::engine::MATLABEngine& engine,
                             matlab::data::ArrayFactory& factory,
                             const std::vector<Eigen::MatrixXcd>& matrices) {

        matlab::data::ArrayDimensions dims{matrices.empty() ? 0U : 1U, matrices.size()};
        matlab::data::TypedArray<matlab::data::Array> output = factory.createCellArray(dims);

        auto write_iter = output.begin();
        for (const auto& matrix : matrices) {
            *write_iter = export_eigen_dense(engine, factory, matrix);
            ++write_iter;
        }

        return output;
    }
}