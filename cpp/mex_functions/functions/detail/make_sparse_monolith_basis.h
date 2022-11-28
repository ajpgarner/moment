/**
 * make_sparse_monolith_basis.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic/symbol_expression.h"
#include "operators/matrix/operator_matrix.h"
#include "operators/matrix/symbol_table.h"

#include "fragments/read_symbol_or_fail.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/visitor.h"

#include "MatlabDataArray.hpp"

#include <complex>

namespace NPATK::mex::functions::detail {

    struct SparseMonolithBasisVisitor {
    public:
        using return_type = std::pair<matlab::data::SparseArray<double>,
                                      matlab::data::SparseArray<std::complex<double>>>;

    private:
        matlab::engine::MATLABEngine &engine;
        const SymbolMatrixProperties &imp;

        template<typename data_t>
        struct monolith_frame {
            std::vector<size_t> index_i{};
            std::vector<size_t> index_j{};
            std::vector<data_t> values{};

            monolith_frame() = default;

            void push_back(size_t i, size_t j, data_t value) {
                index_i.emplace_back(i);
                index_j.emplace_back(j);
                values.emplace_back(value);
            }
        };

        using monolith_re_frame = monolith_frame<double>;
        using monolith_im_frame = monolith_frame<std::complex<double>>;


    public:
        SparseMonolithBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                                   const SymbolMatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /** Dense input -> sparse output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();

            monolith_re_frame real_basis_frame{};
            monolith_im_frame im_basis_frame{};
            const bool hasImBasis = this->imp.is_complex();


            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};
                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        real_basis_frame.push_back(re_id, flatten_index(index_i, index_j), elem.negated ? -1. : 1.);
                        if (index_i != index_j) {
                            real_basis_frame.push_back(re_id, flatten_index(index_j, index_i), elem.negated ? -1. : 1.);
                        }
                    }

                    if (hasImBasis && (im_id>=0)) {
                        assert (index_i != index_j);
                        im_basis_frame.push_back(im_id, flatten_index(index_i, index_j),
                             std::complex<double>{0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                        im_basis_frame.push_back(im_id, flatten_index(index_j, index_i),
                             std::complex<double>{0.0, (elem.negated != elem.conjugated) ? 1. : -1.});
                    }
                }
            }

            return this->construct_basis(real_basis_frame, im_basis_frame);
        }

        /** String (utf16) input -> sparse output */
        return_type string(const matlab::data::StringArray &matrix) {
            const auto& basis_key = this->imp.BasisKey();

            monolith_re_frame real_basis_frame{};
            monolith_im_frame im_basis_frame{};
            const bool symmetric = this->imp.is_hermitian();
            const bool hasImBasis = this->imp.is_complex();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    NPATK::SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};
                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        real_basis_frame.push_back(re_id, flatten_index(index_i, index_j), elem.negated ? -1. : 1.);
                        if (symmetric && (index_i != index_j)) {
                            real_basis_frame.push_back(re_id, flatten_index(index_j, index_i), elem.negated ? -1. : 1.);
                        }
                    }

                    if (hasImBasis && (im_id>=0)) {
                        im_basis_frame.push_back(im_id, flatten_index(index_i, index_j),
                                                 std::complex<double>{0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                        if (symmetric && (index_i != index_j)) {
                            im_basis_frame.push_back(im_id, flatten_index(index_j, index_i),
                                                 std::complex<double>{0.0, (elem.negated != elem.conjugated) ? 1. : -1.});
                        }
                    }
                }
            }
            return this->construct_basis(real_basis_frame, im_basis_frame);
        }

        /** Sparse input -> sparse monolithic output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();

            monolith_re_frame real_basis_frame{};
            monolith_im_frame im_basis_frame{};

            const bool hasImBasis = this->imp.is_complex();
            const bool symmetric = this->imp.is_hermitian();


            auto iter = matrix.cbegin();
            while (iter != matrix.cend()) {
                auto [row, col] = matrix.getIndex(iter);
                if (symmetric && (row > col)) {
                    ++iter;
                    continue;
                }

                NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};

                auto bkIter = basis_key.find(elem.id);
                auto [re_id, im_id] = bkIter->second;

                if (re_id>=0) {
                    real_basis_frame.push_back(re_id, flatten_index(row, col), elem.negated ? -1. : 1.);
                    if (symmetric && (row != col)) {
                        real_basis_frame.push_back(re_id, flatten_index(col, row), elem.negated ? -1. : 1.);
                    }
                }
                if (hasImBasis && (im_id>=0)) {
                    im_basis_frame.push_back(im_id, flatten_index(row, col),
                         std::complex<double>(0.0, (elem.negated != elem.conjugated) ? -1. : 1.));
                    if (symmetric && (row != col)) {
                        im_basis_frame.push_back(im_id, flatten_index(col, row),
                                                 std::complex<double>(0.0, (elem.negated != elem.conjugated) ? 1. : -1.));
                    }
                }
                ++iter;
            }


            return this->construct_basis(real_basis_frame, im_basis_frame);
        }

        /** OperatorMatrix input -> sparse monolithic output */
        return_type operator_matrix(const OperatorMatrix& matrix) {
            const auto& symbols = matrix.Symbols;
            const bool hasImBasis = this->imp.is_complex();
            const bool symmetric = this->imp.is_hermitian();

            monolith_re_frame real_basis_frame{};
            monolith_im_frame im_basis_frame{};

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0 ; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    real_basis_frame.push_back(re_id, flatten_index(index_i, index_j), elem.negated ? -1. : 1.);

                    if (symmetric && (index_i != index_j)) {
                        real_basis_frame.push_back(re_id, flatten_index(index_j, index_i), elem.negated ? -1. : 1.);
                    }

                    if (hasImBasis && (im_id>=0)) {
                        im_basis_frame.push_back(im_id, flatten_index(index_i, index_j),
                             std::complex<double>{0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                        if (symmetric && (index_i != index_j)) {
                            im_basis_frame.push_back(im_id, flatten_index(index_j, index_i),
                                std::complex<double>{0.0, (elem.negated != elem.conjugated) ? 1. : -1.});
                        }
                    }
                }
            }

            return this->construct_basis(
                    symbols.RealSymbolIds().size(), symbols.ImaginarySymbolIds().size(),
                    real_basis_frame, im_basis_frame);
        }

    private:
        return_type construct_basis(const monolith_re_frame& re_frame,
                                    const monolith_im_frame& im_frame) {
            const bool hasImaginaryBasis = this->imp.is_complex();
            const size_t real_mx_cols = this->imp.RealSymbols().size();
            const size_t im_mx_cols = hasImaginaryBasis ? this->imp.ImaginarySymbols().size() : 0;

            return construct_basis(real_mx_cols, im_mx_cols, re_frame, im_frame);
        }

        return_type construct_basis(const size_t real_mx_cols,
                                    const size_t im_mx_cols,
                                    const monolith_re_frame& re_frame,
                                    const monolith_im_frame& im_frame) {
            matlab::data::ArrayFactory factory{};

            const bool hasImaginaryBasis = im_mx_cols > 0;

            // Number of rows is square matrix flattened
            const size_t mx_rows = this->imp.Dimension() * this->imp.Dimension();

            std::pair<size_t, size_t> re_matrix_dims{real_mx_cols, mx_rows};
            std::pair<size_t, size_t> im_matrix_dims{im_mx_cols, mx_rows};

            if (!hasImaginaryBasis) {
                return std::make_pair(
                        make_sparse_matrix<double>(this->engine, re_matrix_dims,
                                                   re_frame.index_i,
                                                   re_frame.index_j,
                                                   re_frame.values),
                        make_zero_sparse_matrix<std::complex<double>>(this->engine, im_matrix_dims));
            }

            return std::make_pair(
                    make_sparse_matrix<double>(this->engine, re_matrix_dims,
                                               re_frame.index_i,
                                               re_frame.index_j,
                                               re_frame.values),
                    make_sparse_matrix<std::complex<double>>(this->engine, im_matrix_dims,
                                               im_frame.index_i,
                                               im_frame.index_j,
                                               im_frame.values));
        }

        [[nodiscard]] constexpr size_t flatten_index(size_t index_i, size_t index_j) const noexcept {
            return (index_j * this->imp.Dimension()) + index_i;
        }

    };

    static_assert(concepts::VisitorHasRealDense<SparseMonolithBasisVisitor>);
    static_assert(concepts::VisitorHasRealSparse<SparseMonolithBasisVisitor>);
    static_assert(concepts::VisitorHasString<SparseMonolithBasisVisitor>);


    inline auto make_sparse_monolith_basis(matlab::engine::MATLABEngine &engine,
                                           const matlab::data::Array &input,
                                           const SymbolMatrixProperties &imp) {
        // Get symbols in matrix...
        return DispatchVisitor(engine, input, SparseMonolithBasisVisitor{engine, imp});
    }


    inline auto make_sparse_monolith_basis(matlab::engine::MATLABEngine &engine,
                                          const OperatorMatrix& mm) {
        SparseMonolithBasisVisitor smbv{engine, mm.SMP()};
        return smbv.operator_matrix(mm);
    }
}