/**
 * make_sparse_cell_basis.h
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

    struct SparseCellBasisVisitor {
    public:
        using return_type = std::pair<matlab::data::CellArray, matlab::data::CellArray>;

    private:
        matlab::engine::MATLABEngine &engine;
        const SymbolMatrixProperties &imp;

        struct sparse_basis_re_frame {
            std::vector<size_t> index_i{};
            std::vector<size_t> index_j{};
            std::vector<double> values{};

            void push_back(size_t i, size_t j, double value) {
                index_i.emplace_back(i);
                index_j.emplace_back(j);
                values.emplace_back(value);

                if (i != j) {
                    index_i.emplace_back(j);
                    index_j.emplace_back(i);
                    values.emplace_back(value);
                }
            }
        };

        struct sparse_basis_im_frame {
            std::vector<size_t> index_i{};
            std::vector<size_t> index_j{};
            std::vector<std::complex<double>> values;

            void push_back(size_t i, size_t j, std::complex<double> value) {
                index_i.emplace_back(i);
                index_j.emplace_back(j);
                values.emplace_back(value);

                if (i != j) {
                    index_i.emplace_back(j);
                    index_j.emplace_back(i);
                    values.emplace_back(conj(value));
                }
            }
        };

    public:
        SparseCellBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                                const SymbolMatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}


        /** Dense input -> sparse output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type dense(const matlab::data::TypedArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();
            std::vector<sparse_basis_re_frame> real_basis_frame(this->imp.RealSymbols().size());
            std::vector<sparse_basis_im_frame> im_basis_frame(this->imp.ImaginarySymbols().size());

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    NPATK::SymbolExpression elem{static_cast<symbol_name_t>(matrix[index_i][index_j])};

                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id>=0) {
                        assert(re_id < real_basis_frame.size());
                        real_basis_frame[re_id].push_back(index_i, index_j, elem.negated ? -1. : 1.);
                    }

                    if (im_id>=0) {
                        assert(im_id < im_basis_frame.size());
                        im_basis_frame[im_id].push_back(index_i, index_j, std::complex<double>{
                                0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                    }
                }
            }

            return construct_basis(real_basis_frame, im_basis_frame);
        }

        /** String (utf16) input -> sparse output */
        return_type string(const matlab::data::StringArray &matrix) {
            const auto& basis_key = this->imp.BasisKey();
            std::vector<sparse_basis_re_frame> real_basis_frame(this->imp.RealSymbols().size());
            std::vector<sparse_basis_im_frame> im_basis_frame(this->imp.ImaginarySymbols().size());

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    SymbolExpression elem{read_symbol_or_fail(engine, matrix, index_i, index_j)};

                    auto bkIter = basis_key.find(elem.id);
                    auto [re_id, im_id] = bkIter->second;

                    if (re_id >= 0) {
                        assert(re_id < real_basis_frame.size());
                        real_basis_frame[re_id].push_back(index_i, index_j, elem.negated ? -1. : 1.);
                    }

                    if (im_id >= 0) {
                        assert(im_id < im_basis_frame.size());
                        im_basis_frame[im_id].push_back(index_i, index_j, std::complex<double>{
                                0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                    }
                }
            }
            return construct_basis(real_basis_frame, im_basis_frame);
        }

        /** Sparse input -> sparse output */
        template<std::convertible_to<symbol_name_t> data_t>
        return_type sparse(const matlab::data::SparseArray<data_t> &matrix) {
            const auto& basis_key = this->imp.BasisKey();
            std::vector<sparse_basis_re_frame> real_basis_frame(this->imp.RealSymbols().size());
            std::vector<sparse_basis_im_frame> im_basis_frame(this->imp.ImaginarySymbols().size());

            auto iter = matrix.cbegin();
            while (iter != matrix.cend()) {
                auto [row, col] = matrix.getIndex(iter);
                if (row > col) {
                    ++iter;
                    continue;
                }

                NPATK::SymbolExpression elem{static_cast<symbol_name_t>(*iter)};

                auto bkIter = basis_key.find(elem.id);
                auto [re_id, im_id] = bkIter->second;

                if (re_id>=0) {
                    assert(re_id < real_basis_frame.size());
                    real_basis_frame[re_id].push_back(row, col,
                                                      elem.negated ? -1. : 1.);
                }
                if (im_id>=0) {
                    assert(im_id < im_basis_frame.size());
                    im_basis_frame[im_id].push_back(row, col, std::complex<double>(
                            0.0, (elem.negated != elem.conjugated) ? -1. : 1.));

                }
                ++iter;
            }


            return construct_basis(real_basis_frame, im_basis_frame);
        }

        /** Moment matrix input -> sparse output */
        return_type operator_matrix(const OperatorMatrix& matrix) {
            const auto& symbols = matrix.Symbols;

            std::vector<sparse_basis_re_frame> real_basis_frame(symbols.RealSymbolIds().size());
            std::vector<sparse_basis_im_frame> im_basis_frame(symbols.ImaginarySymbolIds().size());

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = index_i; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        assert(re_id < real_basis_frame.size());
                        real_basis_frame[re_id].push_back(index_i, index_j, elem.negated ? -1. : 1.);
                    }

                    if (im_id>=0) {
                        assert(im_id < im_basis_frame.size());
                        im_basis_frame[im_id].push_back(index_i, index_j, std::complex<double>{
                                0.0, (elem.negated != elem.conjugated) ? -1. : 1.});
                    }
                }
            }
            return construct_basis(real_basis_frame, im_basis_frame);
        }


    private:
        return_type construct_basis(const std::vector<sparse_basis_re_frame>& re_frame,
                                    const std::vector<sparse_basis_im_frame>& im_frame) {
            matlab::data::ArrayFactory factory{};
            matlab::data::ArrayDimensions re_ad{re_frame.empty() ? 0U : 1U,
                                                re_frame.size()};
            matlab::data::ArrayDimensions im_ad{im_frame.empty() ? 0U : 1U,
                                                im_frame.size()};
            return_type output = std::make_pair(factory.createArray<matlab::data::Array>(re_ad),
                                                  factory.createArray<matlab::data::Array>(im_ad));

            for (size_t re_id = 0, re_max = re_frame.size(); re_id < re_max; ++re_id) {

                output.first[0][re_id] = make_sparse_matrix<double>(this->engine,
                                                                    std::make_pair(this->imp.Dimension(),
                                                                                   this->imp.Dimension()),
                                                                    re_frame[re_id].index_i,
                                                                    re_frame[re_id].index_j,
                                                                    re_frame[re_id].values);
            }

            for (size_t im_id = 0, im_max = im_frame.size(); im_id < im_max; ++im_id) {
                output.second[0][im_id] = make_sparse_matrix<std::complex<double>>(this->engine,
                        std::make_pair(this->imp.Dimension(),
                                       this->imp.Dimension()),
                        im_frame[im_id].index_i,
                        im_frame[im_id].index_j,
                        im_frame[im_id].values);
            }
            return output;
        }
    };

    static_assert(concepts::VisitorHasRealDense<SparseCellBasisVisitor>);
    static_assert(concepts::VisitorHasRealSparse<SparseCellBasisVisitor>);
    static_assert(concepts::VisitorHasString<SparseCellBasisVisitor>);

    inline auto make_sparse_cell_basis(matlab::engine::MATLABEngine &engine,
                           const matlab::data::Array &input,
                           const SymbolMatrixProperties &imp) {
        // Get symbols in matrix...
        return DispatchVisitor(engine, input, SparseCellBasisVisitor{engine, imp});
    }


    inline auto make_sparse_cell_basis(matlab::engine::MATLABEngine &engine,
                                      const OperatorMatrix& mm) {
        SparseCellBasisVisitor scbv{engine, mm.SMP()};
        return scbv.operator_matrix(mm);
    }
}