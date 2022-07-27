/**
 * make_dense_cell_basis_visitor.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol_expression.h"
#include "symbolic/index_matrix_properties.h"
#include "operators/moment_matrix.h"

#include "fragments/read_symbol_or_fail.h"

#include "utilities/visitor.h"

#include "MatlabDataArray.hpp"

#include <complex>

namespace NPATK::mex::functions::detail {
    struct DenseCellBasisVisitor {
    private:
        matlab::engine::MATLABEngine &engine;
        const IndexMatrixProperties &imp;

    public:
        using return_type = std::pair<matlab::data::CellArray, matlab::data::CellArray>;

        DenseCellBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                              const IndexMatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}


        /** Dense input -> dense output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
            auto output = create_empty_basis();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                        re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                    }
                    if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {
                        matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                        im_mat[index_i][index_j] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        im_mat[index_j][index_i] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
            }

            return output;
        }


        /** String input -> dense output */
        return_type string(const matlab::data::StringArray& matrix) {
            auto output = create_empty_basis();
            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                        re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                    }
                    if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) &&  (im_id>=0)) {
                        matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                        im_mat[index_i][index_j] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        im_mat[index_j][index_i] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
            }

            return output;
        }

        /** Sparse input -> dense output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {
            auto output = create_empty_basis();

            auto iter = matrix.cbegin();
            while (iter != matrix.cend()) {
                auto indices = matrix.getIndex(iter);

                if (indices.first > indices.second) {
                    ++iter;
                    continue;
                }

                NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};
                auto [re_id, im_id] = this->imp.BasisKey(elem.id);
                if (re_id>=0) {
                    matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                    re_mat[indices.first][indices.second] = elem.negated ? -1 : 1;
                    re_mat[indices.second][indices.first] = elem.negated ? -1 : 1;
                }
                if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {
                    matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                    im_mat[indices.first][indices.second] = std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                    im_mat[indices.second][indices.first] = std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                }
                ++iter;
            }

            return output;
        }

        /* Moment matrix input -> dense output */
        return_type moment_matrix(const MomentMatrix& matrix) {
            auto output = create_empty_basis();
            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                        re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                    }

                    if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {
                        matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                        im_mat[index_i][index_j] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        im_mat[index_j][index_i] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
            }

            return output;
        }

    private:

        return_type create_empty_basis() {
            matlab::data::ArrayFactory factory{};

            matlab::data::ArrayDimensions re_ad{this->imp.RealSymbols().empty() ? 0U : 1U,
                                                this->imp.RealSymbols().size()};
            matlab::data::ArrayDimensions im_ad{this->imp.ImaginarySymbols().empty() ? 0U : 1U,
                                                this->imp.ImaginarySymbols().size()};

            auto output = std::make_pair(factory.createCellArray(re_ad),
                                         factory.createCellArray(im_ad));

            for (size_t rr = 0, rr_max = this->imp.RealSymbols().size(); rr < rr_max; ++rr) {
                output.first[rr] = factory.createArray<double>({this->imp.Dimension(), this->imp.Dimension()});
            }

            if (this->imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) {
                for (size_t ri = 0, ri_max = this->imp.ImaginarySymbols().size(); ri < ri_max; ++ri) {
                    output.second[ri] = factory.createArray<std::complex<double>>({this->imp.Dimension(), this->imp.Dimension()});
                }
            }
            return output;
        }

    };

    static_assert(concepts::VisitorHasRealDense < DenseCellBasisVisitor >);
    static_assert(concepts::VisitorHasRealSparse < DenseCellBasisVisitor >);
    static_assert(concepts::VisitorHasString < DenseCellBasisVisitor >);


    inline auto make_dense_cell_basis(matlab::engine::MATLABEngine &engine,
                          const matlab::data::Array &input,
                          const IndexMatrixProperties &imp) {
        // Get symbols in matrix...
        return DispatchVisitor(engine, input, DenseCellBasisVisitor{engine, imp});
    }

    inline auto make_dense_cell_basis(matlab::engine::MATLABEngine &engine,
                                  const MomentMatrix& mm) {
        // Get symbols in matrix...
        DenseCellBasisVisitor mdbv{engine, mm.BasisIndices()};
        return mdbv.moment_matrix(mm);

    }
}