/**
 * export_basis.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "export_basis.h"

#include "matrix/matrix.h"

#include "eigen/export_eigen_dense.h"
#include "eigen/export_eigen_sparse.h"

#include <complex>

namespace Moment::mex {

    namespace {

        std::pair<matlab::data::CellArray, matlab::data::CellArray>
        export_dense_basis(matlab::engine::MATLABEngine& engine,
                           matlab::data::ArrayFactory& factory,
                           const Matrix &mm) {

            const auto &[re_basis, im_basis] = mm.Basis.Dense();

            return std::make_pair(export_eigen_dense_array(engine, factory, re_basis),
                                  export_eigen_dense_array(engine, factory, im_basis));

        }

        std::pair<matlab::data::CellArray, matlab::data::CellArray>
        export_dense_complex_basis(matlab::engine::MATLABEngine &engine,
                                   matlab::data::ArrayFactory& factory,
                                   const Matrix &mm) {

            const auto &[re_basis, im_basis] = mm.Basis.DenseComplex();

            return std::make_pair(export_eigen_dense_array(engine, factory, re_basis),
                                  export_eigen_dense_array(engine, factory, im_basis));

        }

        std::pair<matlab::data::CellArray, matlab::data::CellArray>
        export_sparse_basis(matlab::engine::MATLABEngine &engine,
                            matlab::data::ArrayFactory& factory,
                            const Matrix &mm) {
            const auto &[re_basis, im_basis] = mm.Basis.Sparse();

            return std::make_pair(export_eigen_sparse_array(engine, factory, re_basis),
                                  export_eigen_sparse_array(engine, factory, im_basis));
        }

        std::pair<matlab::data::CellArray, matlab::data::CellArray>
        export_sparse_complex_basis(matlab::engine::MATLABEngine &engine,
                                    matlab::data::ArrayFactory& factory,
                                 const Matrix &mm) {
            const auto &[re_basis, im_basis] = mm.Basis.SparseComplex();

            return std::make_pair(export_eigen_sparse_array(engine, factory, re_basis),
                                  export_eigen_sparse_array(engine, factory, im_basis));
        }

        std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>
        export_dense_monolith_basis(matlab::engine::MATLABEngine &engine,
                                    matlab::data::ArrayFactory& factory,
                                    const Matrix &mm) {
            const auto &[re_basis, im_basis] = mm.Basis.DenseMonolithic();

            return std::make_pair(export_eigen_dense(engine, factory, re_basis),
                                  export_eigen_dense(engine, factory, im_basis));

        }

        std::pair<matlab::data::TypedArray<std::complex<double>>, matlab::data::TypedArray<std::complex<double>>>
        export_dense_monolith_complex_basis(matlab::engine::MATLABEngine &engine,
                                            matlab::data::ArrayFactory& factory,
                                            const Matrix &mm) {
            const auto &[re_basis, im_basis] = mm.Basis.DenseMonolithicComplex();

            return std::make_pair(export_eigen_dense(engine, factory, re_basis),
                                  export_eigen_dense(engine, factory, im_basis));

        }

        std::pair<matlab::data::SparseArray<double>, matlab::data::SparseArray<std::complex<double>>>
        export_sparse_monolith_basis(matlab::engine::MATLABEngine &engine,
                                     matlab::data::ArrayFactory& factory,
                                     const Matrix &mm) {
            const auto &[re_basis, im_basis] = mm.Basis.SparseMonolithic();

            return std::make_pair(export_eigen_sparse(engine, factory, re_basis),
                                  export_eigen_sparse(engine, factory, im_basis));
        }

        std::pair<matlab::data::SparseArray<std::complex<double>>, matlab::data::SparseArray<std::complex<double>>>
        export_sparse_monolith_complex_basis(matlab::engine::MATLABEngine &engine,
                                             matlab::data::ArrayFactory& factory,
                                             const Matrix &mm) {
            const auto &[re_basis, im_basis] = mm.Basis.SparseMonolithicComplex();

            return std::make_pair(export_eigen_sparse(engine, factory, re_basis),
                                  export_eigen_sparse(engine, factory, im_basis));
        }
    }

    std::pair<matlab::data::Array, matlab::data::Array> BasisExporter::operator()(const Matrix &matrix) const {
        const auto real_coefficients = !matrix.HasComplexCoefficients();

        if (!this->Monolithic) {
            if (!this->Sparse) {
                if (real_coefficients) {
                    // C / D / RC
                    return export_dense_basis(this->engine, this->factory, matrix);
                } else {
                    // C / D / CC
                    return export_dense_complex_basis(this->engine, this->factory, matrix);
                }
            } else {
                if (real_coefficients) {
                    // C / S / RC
                    return export_sparse_basis(this->engine, this->factory, matrix);
                } else {
                    // C / S / CC
                    return export_sparse_complex_basis(this->engine, this->factory, matrix);
                }
            }
        } else {
            if (!this->Sparse) {
                if (real_coefficients) {
                    // M / D / RC
                    return export_dense_monolith_basis(this->engine, this->factory, matrix);
                } else {
                    // M / D / CC
                    return export_dense_monolith_complex_basis(this->engine, this->factory, matrix);
                }
            } else {
                if (real_coefficients) {
                    // M / S / RC
                    return export_sparse_monolith_basis(this->engine, this->factory, matrix);
                } else {
                    // M / S / CC
                    return export_sparse_monolith_complex_basis(this->engine, this->factory, matrix);
                }
            }
        }

    }
}