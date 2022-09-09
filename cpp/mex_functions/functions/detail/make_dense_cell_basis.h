/**
 * make_dense_cell_basis_visitor.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol_expression.h"
#include "operators/matrix/operator_matrix.h"
#include "operators/matrix/symbol_table.h"

#include "fragments/read_symbol_or_fail.h"
#include "utilities/visitor.h"

#include "MatlabDataArray.hpp"

#include <complex>

namespace NPATK::mex::functions::detail {
    struct DenseCellBasisVisitor {
    private:
        matlab::engine::MATLABEngine &engine;
        const SymbolMatrixProperties &imp;

    public:
        using return_type = std::pair<matlab::data::CellArray, matlab::data::CellArray>;

        DenseCellBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                              const SymbolMatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}


        /** Dense input -> dense output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();
            auto output = create_empty_basis();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};

                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                        re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                    }
                    if ((imp.Type() == MatrixType::Hermitian) && (im_id>=0)) {
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
            const auto& basis_key = this->imp.BasisKey();
            auto output = create_empty_basis();
            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};

                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                        re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                    }
                    if ((imp.Type() == MatrixType::Hermitian) &&  (im_id>=0)) {
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
            const auto& basis_key = this->imp.BasisKey();
            auto output = create_empty_basis();

            auto iter = matrix.cbegin();
            while (iter != matrix.cend()) {
                auto indices = matrix.getIndex(iter);

                if (indices.first > indices.second) {
                    ++iter;
                    continue;
                }

                NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};

                auto bkIter = basis_key.find(elem.id);
                auto [re_id, im_id] = bkIter->second;
                if (re_id>=0) {
                    matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                    re_mat[indices.first][indices.second] = elem.negated ? -1 : 1;
                    re_mat[indices.second][indices.first] = elem.negated ? -1 : 1;
                }
                if ((imp.Type() == MatrixType::Hermitian) && (im_id>=0)) {
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
        return_type operator_matrix(const OperatorMatrix& matrix) {
            const auto& symbols = matrix.Symbols;
            auto output = create_empty_basis(symbols);

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.negated ? -1 : 1;
                        re_mat[index_j][index_i] = elem.negated ? -1 : 1;
                    }

                    if ((imp.Type() == MatrixType::Hermitian) && (im_id>=0)) {
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
            const bool hasImaginaryBasis = (this->imp.Type() == MatrixType::Hermitian);
            return create_empty_basis(this->imp.RealSymbols().size(),
                                      hasImaginaryBasis ? this->imp.ImaginarySymbols().size() : 0,
                                      this->imp.Dimension());
        }

        return_type create_empty_basis(const SymbolTable& table) {
            return create_empty_basis(table.RealSymbolIds().size(), table.ImaginarySymbolIds().size(),
                                      this->imp.Dimension());
        }

        static return_type create_empty_basis(const size_t real_elems, const size_t im_elems, const size_t dimension) {
            matlab::data::ArrayFactory factory{};

            matlab::data::ArrayDimensions re_ad{real_elems > 0 ? 0U : 1U, real_elems};
            matlab::data::ArrayDimensions im_ad{im_elems > 0 ? 0U : 1U, im_elems};

            auto output = std::make_pair(factory.createCellArray(re_ad),
                                         factory.createCellArray(im_ad));

            for (size_t rr = 0; rr < real_elems; ++rr) {
                output.first[rr] = factory.createArray<double>({dimension, dimension});
            }

            for (size_t ri = 0; ri < im_elems; ++ri) {
                output.second[ri] = factory.createArray<std::complex<double>>({dimension, dimension});
            }

            return output;
        }
    };

    static_assert(concepts::VisitorHasRealDense<DenseCellBasisVisitor>);
    static_assert(concepts::VisitorHasRealSparse<DenseCellBasisVisitor>);
    static_assert(concepts::VisitorHasString<DenseCellBasisVisitor>);


    inline auto make_dense_cell_basis(matlab::engine::MATLABEngine &engine,
                          const matlab::data::Array &input,
                          const SymbolMatrixProperties &imp) {
        // Get symbols in matrix...
        return DispatchVisitor(engine, input, DenseCellBasisVisitor{engine, imp});
    }

    inline auto make_dense_cell_basis(matlab::engine::MATLABEngine &engine,
                                           const OperatorMatrix& mm) {
        DenseCellBasisVisitor dcbv{engine, mm.SMP()};
        return dcbv.operator_matrix(mm);
    }
}