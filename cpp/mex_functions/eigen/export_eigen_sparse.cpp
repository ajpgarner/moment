/**
 * export_eigen_sparse.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_eigen_sparse.h"

#include "error_codes.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/reporting.h"

namespace Moment::mex {
    matlab::data::SparseArray<double> export_eigen_sparse(matlab::engine::MATLABEngine& engine,
                                                          const Eigen::SparseMatrix<double>& matrix) {

        matlab::data::ArrayDimensions dims{static_cast<size_t>(matrix.rows()),
                                           static_cast<size_t>(matrix.cols())};
        const auto nnz = static_cast<size_t>(matrix.nonZeros());

        // Special case for empty matrix
        if (nnz == 0) {
            return make_zero_sparse_matrix<double>(engine, {dims[0], dims[1]});
        }

        // Otherwise, do properly
        matlab::data::ArrayFactory factory;

        auto rows_p = factory.createBuffer<size_t>(nnz);
        auto cols_p = factory.createBuffer<size_t>(nnz);
        auto data_p = factory.createBuffer<double>(nnz);

        // Write data into the buffers
        size_t total_index = 0;


        for (int k=0; k < matrix.outerSize(); ++k) {
            for (Eigen::SparseMatrix<double>::InnerIterator inner_iter{matrix,k}; inner_iter; ++inner_iter) {
                assert(total_index < nnz);
                rows_p[total_index] = static_cast<size_t>(inner_iter.row());
                cols_p[total_index] = static_cast<size_t>(inner_iter.col());
                data_p[total_index] = static_cast<double>(inner_iter.value());
                ++total_index;
            }
        }
        assert(total_index == nnz);

        return factory.createSparseArray<double>(dims, nnz,
                                                 std::move(data_p), std::move(rows_p), std::move(cols_p));
    }
}