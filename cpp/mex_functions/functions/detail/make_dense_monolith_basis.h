/**
 * make_dense_monolith_basis.h
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

namespace Moment::mex::functions::detail {

    struct DenseMonolithBasisVisitor {
    private:
        matlab::engine::MATLABEngine &engine;
        const SymbolMatrixProperties &imp;
    public:
        using return_type = std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>;

        DenseMonolithBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                              const SymbolMatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /** Dense input -> dense monolithic output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();
            auto output = create_empty_basis();
            const bool symmetric = this->imp.is_hermitian();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    Moment::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.negated ? -1 : 1;
                        if (symmetric && (index_i != index_j)) {
                            output.first[re_id][flatten_index(index_j, index_i)] = elem.negated ? -1 : 1;
                        }
                    }
                    if ((imp.is_complex()) && (im_id>=0)) {
                        output.second[im_id][flatten_index(index_i, index_j)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        if (symmetric && (index_i != index_j)) {
                            output.second[im_id][flatten_index(index_j, index_i)] = std::complex<double>{
                                    0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                        }
                    }
                }
            }

            return output;
        }

        /** String input -> dense monolithic output */
        return_type string(const matlab::data::StringArray& matrix) {
            const auto& basis_key = this->imp.BasisKey();
            auto output = create_empty_basis();
            const bool symmetric = this->imp.is_hermitian();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.negated ? -1 : 1;
                        if (symmetric && (index_i != index_j)) {
                            output.first[re_id][flatten_index(index_j, index_i)] = elem.negated ? -1 : 1;
                        }
                    }
                    if ((this->imp.is_complex()) && (im_id>=0)) {
                        output.second[im_id][flatten_index(index_i, index_j)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        if (symmetric && (index_i != index_j)) {
                            output.second[im_id][flatten_index(index_j, index_i)] =  std::complex<double>{
                                    0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                        }
                    }
                }
            }
            return output;
        }

        /** Sparse input -> dense monolithic output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();
            auto output = create_empty_basis();
            const bool symmetric = this->imp.is_hermitian();
            auto iter = matrix.cbegin();
            while (iter != matrix.cend()) {
                auto indices = matrix.getIndex(iter);

                if (symmetric && (indices.first > indices.second)) {
                    ++iter;
                    continue;
                }

                Moment::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};
                auto bkIter = basis_key.find(elem.id);
                auto [re_id, im_id] = bkIter->second;
                if (re_id>=0) {
                    output.first[re_id][flatten_index(indices.first, indices.second)] = elem.negated ? -1 : 1;
                    if (symmetric && (indices.first != indices.second)) {
                        output.first[re_id][flatten_index(indices.second, indices.first)] = elem.negated ? -1 : 1;
                    }
                }
                if ((this->imp.is_complex()) && (im_id>=0)) {
                    output.second[im_id][flatten_index(indices.first, indices.second)] = std::complex<double>{
                            0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                    if (symmetric && (indices.first != indices.second)) {
                        output.second[im_id][flatten_index(indices.second, indices.first)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                    }
                }
                ++iter;
            }

            return output;
        }

        /** Moment matrix input -> dense monolithic output */
        return_type operator_matrix(const OperatorMatrix& matrix) {
            const auto& symbols = matrix.Symbols;
            const bool symmetric = this->imp.is_hermitian();
            auto output = create_empty_basis(symbols);

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.negated ? -1 : 1;
                        if (symmetric && (index_i != index_j)) {
                            output.first[re_id][flatten_index(index_j, index_i)] = elem.negated ? -1 : 1;
                        }
                    }

                    if ((this->imp.is_complex()) && (im_id>=0)) {

                        output.second[im_id][flatten_index(index_i, index_j)] = std::complex<double>{
                                0.0, (elem.conjugated != elem.negated) ? -1. : 1.};
                        if (symmetric && (index_i != index_j)) {
                            output.second[im_id][flatten_index(index_j, index_i)] = std::complex<double>{
                                    0.0, (elem.conjugated != elem.negated) ? 1. : -1.};
                        }
                    }
                }
            }


            return output;
        }

    private:
        return_type create_empty_basis() {
            return create_empty_basis(this->imp.RealSymbols().size(),
                                      this->imp.is_complex() ? this->imp.ImaginarySymbols().size() : 0,
                                      this->imp.Dimension());
        }

        return_type create_empty_basis(const SymbolTable& table) {
            return create_empty_basis(table.RealSymbolIds().size(), table.ImaginarySymbolIds().size(),
                                      this->imp.Dimension());
        }

        static return_type create_empty_basis(size_t real_mx_cols, size_t im_mx_cols, size_t dimension) {
            matlab::data::ArrayFactory factory{};

            // Number of rows is square matrix flattened
            const size_t mx_rows = dimension * dimension;

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
                                       const SymbolMatrixProperties &imp) {
        // Get symbols in matrix...
        return DispatchVisitor(engine, input, DenseMonolithBasisVisitor{engine, imp});
    }


    inline auto make_dense_monolith_basis(matlab::engine::MATLABEngine &engine,
                                                  const OperatorMatrix& mm) {
        DenseMonolithBasisVisitor dmbv{engine, mm.SMP()};
        return dmbv.operator_matrix(mm);
    }

}