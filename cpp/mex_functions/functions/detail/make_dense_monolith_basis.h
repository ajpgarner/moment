/**
 * make_dense_monolith_basis.h
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

    struct DenseMonolithBasisVisitor {
    private:
        matlab::engine::MATLABEngine &engine;
        const IndexMatrixProperties &imp;

    public:
        using return_type = std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>;

        DenseMonolithBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                              const IndexMatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /** Dense input -> dense monolithic output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
            auto output = create_empty_basis();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.negated ? -1 : 1;
                        output.first[re_id][flatten_index(index_j, index_i)] = elem.negated ? -1 : 1;
                    }
                    if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {
                        output.second[im_id][flatten_index(index_i, index_j)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        output.second[im_id][flatten_index(index_j, index_i)] =  std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
            }

            return output;
        }

        /** String input -> dense monolithic output */
        return_type string(const matlab::data::StringArray& matrix) {
            auto output = create_empty_basis();
            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.negated ? -1 : 1;
                        output.first[re_id][flatten_index(index_j, index_i)] = elem.negated ? -1 : 1;
                    }
                    if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {
                        output.second[im_id][flatten_index(index_i, index_j)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        output.second[im_id][flatten_index(index_j, index_i)] =  std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
            }
            return output;
        }

        /** Sparse input -> dense monolithic output */
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
                    output.first[re_id][flatten_index(indices.first, indices.second)] = elem.negated ? -1 : 1;
                    output.first[re_id][flatten_index(indices.second, indices.first)] = elem.negated ? -1 : 1;
                }
                if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {

                    output.second[im_id][flatten_index(indices.first, indices.second)] = std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                    output.second[im_id][flatten_index(indices.second, indices.first)] =  std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                }
                ++iter;
            }

            return output;
        }

        /** Moment matrix input -> dense monolithic output */
        return_type moment_matrix(const MomentMatrix& matrix) {
            auto output = create_empty_basis();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = this->imp.BasisKey(elem.id);

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.negated ? -1 : 1;
                        output.first[re_id][flatten_index(index_j, index_i)] = elem.negated ? -1 : 1;
                    }

                    if ((imp.Type() == IndexMatrixProperties::MatrixType::Hermitian) && (im_id>=0)) {

                        output.second[im_id][flatten_index(index_i, index_j)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        output.second[im_id][flatten_index(index_j, index_i)] =  std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
            }


            return output;
        }

    private:

        return_type create_empty_basis() {
            matlab::data::ArrayFactory factory{};
            const bool hasImaginaryBasis = (this->imp.Type() == IndexMatrixProperties::MatrixType::Hermitian);

            // Number of columns* corresponds to number of symbols [* matlab is col-major]
            const size_t real_mx_cols = this->imp.RealSymbols().size();
            const size_t im_mx_cols = hasImaginaryBasis ? this->imp.ImaginarySymbols().size() : 0;

            // Number of rows is square matrix flattened
            const size_t mx_rows = this->imp.Dimension() * this->imp.Dimension();

            matlab::data::ArrayDimensions re_matrix{real_mx_cols, mx_rows};
            matlab::data::ArrayDimensions im_matrix{im_mx_cols, mx_rows};

            return std::make_pair(factory.createArray<double>(re_matrix),
                                  factory.createArray<std::complex<double>>(im_matrix));
        }

        [[nodiscard]] constexpr size_t flatten_index(size_t index_i, size_t index_j) const noexcept {
            return (index_j * this->imp.Dimension()) + index_i;
        }


    };

    static_assert(concepts::VisitorHasRealDense<DenseMonolithBasisVisitor>);
    static_assert(concepts::VisitorHasRealSparse<DenseMonolithBasisVisitor>);
    static_assert(concepts::VisitorHasString<DenseMonolithBasisVisitor>);


    inline auto make_dense_monolith_basis(matlab::engine::MATLABEngine &engine,
                                       const matlab::data::Array &input,
                                       const IndexMatrixProperties &imp) {
        // Get symbols in matrix...
        return DispatchVisitor(engine, input, DenseMonolithBasisVisitor{engine, imp});
    }


    inline auto make_dense_monolith_basis(matlab::engine::MATLABEngine &engine,
                                       const MomentMatrix& mm) {
        // Get symbols in matrix...
        DenseMonolithBasisVisitor mdbv{engine, mm.BasisIndices()};
        return mdbv.moment_matrix(mm);
    }
}