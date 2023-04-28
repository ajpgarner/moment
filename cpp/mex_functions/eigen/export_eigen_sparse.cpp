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

    namespace {
        template<typename scalar_t>
        inline matlab::data::SparseArray<scalar_t>
        do_export_eigen_sparse(matlab::engine::MATLABEngine &engine,
                               matlab::data::ArrayFactory &factory,
                               const Eigen::SparseMatrix<scalar_t> &matrix) {

            matlab::data::ArrayDimensions dims{static_cast<size_t>(matrix.rows()),
                                               static_cast<size_t>(matrix.cols())};
            const auto nnz = static_cast<size_t>(matrix.nonZeros());

            // Special case for empty matrix
            if (nnz == 0) {
                return make_zero_sparse_matrix<scalar_t>(engine, {dims[0], dims[1]});
            }

            // Otherwise, do properly
            auto rows_p = factory.createBuffer<size_t>(nnz);
            auto cols_p = factory.createBuffer<size_t>(nnz);
            auto data_p = factory.createBuffer<scalar_t>(nnz);

            // Write data into the buffers
            size_t total_index = 0;


            for (int k = 0; k < matrix.outerSize(); ++k) {
                for (typename Eigen::SparseMatrix<scalar_t>::InnerIterator inner_iter{matrix, k}; inner_iter; ++inner_iter) {
                    assert(total_index < nnz);
                    rows_p[total_index] = static_cast<size_t>(inner_iter.row());
                    cols_p[total_index] = static_cast<size_t>(inner_iter.col());
                    data_p[total_index] = static_cast<scalar_t>(inner_iter.value());
                    ++total_index;
                }
            }
            assert(total_index == nnz);

            return factory.createSparseArray<scalar_t>(dims, nnz,
                                                     std::move(data_p), std::move(rows_p), std::move(cols_p));
        }
    }

    matlab::data::SparseArray<double>
    export_eigen_sparse(matlab::engine::MATLABEngine &engine,
                           matlab::data::ArrayFactory &factory,
                           const Eigen::SparseVector<double>& matrix) {
        return do_export_eigen_sparse<double>(engine, factory, matrix);
    }

    matlab::data::SparseArray<double>
    export_eigen_sparse(matlab::engine::MATLABEngine &engine,
                           matlab::data::ArrayFactory &factory,
                           const Eigen::SparseMatrix<double> &matrix) {
        return do_export_eigen_sparse<double>(engine, factory, matrix);
    }

    matlab::data::SparseArray<std::complex<double>>
    export_eigen_sparse(matlab::engine::MATLABEngine &engine,
                           matlab::data::ArrayFactory &factory,
                           const Eigen::SparseMatrix<std::complex<double>> &matrix) {
        return do_export_eigen_sparse<std::complex<double>>(engine, factory, matrix);
    }

    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_sparse_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::SparseMatrix<double>>& matrices) {

        matlab::data::ArrayDimensions dims{1, matrices.size()};
        matlab::data::TypedArray<matlab::data::Array> output = factory.createCellArray(dims);
        auto write_iter = output.begin();
        for (const auto& matrix : matrices) {
            *write_iter = export_eigen_sparse(engine, factory, matrix);
            ++write_iter;
        }

        return output;
    }

    matlab::data::TypedArray<matlab::data::Array>
    export_eigen_sparse_array(matlab::engine::MATLABEngine& engine,
                              matlab::data::ArrayFactory& factory,
                              const std::vector<Eigen::SparseMatrix<std::complex<double>>>& matrices) {

        matlab::data::ArrayDimensions dims{1, matrices.size()};
        matlab::data::TypedArray<matlab::data::Array> output = factory.createCellArray(dims);
        auto write_iter = output.begin();
        for (const auto& matrix : matrices) {
            *write_iter = export_eigen_sparse(engine, factory, matrix);
            ++write_iter;
        }

        return output;
    }

}