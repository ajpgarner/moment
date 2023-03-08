/**
 * export_eigen_dense.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_eigen_dense.h"

#include "error_codes.h"
#include "utilities/reporting.h"

namespace Moment::mex {
    matlab::data::TypedArray<double> export_eigen_dense(matlab::engine::MATLABEngine& engine,
                                                        const Eigen::MatrixXd& matrix) {
        const matlab::data::ArrayDimensions dims{static_cast<size_t>(matrix.rows()),
                                           static_cast<size_t>(matrix.cols())};

        matlab::data::ArrayFactory factory;
        auto output = factory.createArray<double>(dims);

        // MATLAB is col major
        auto write_iter = output.begin();
        for (size_t col = 0; col < dims[1]; ++col) {
            for (size_t row = 0; row < dims[0]; ++row) {
                *write_iter = matrix(row, col);
                ++write_iter;
            }
        }
        assert(write_iter == output.end());

        return output;
    }
}