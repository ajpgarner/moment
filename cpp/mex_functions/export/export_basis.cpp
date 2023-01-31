/**
 * export_basis.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "export_basis.h"

#include "symbolic/symbol_expression.h"

#include "matrix/operator_matrix.h"

#include "symbolic/symbol_table.h"

#include "import/read_symbol_or_fail.h"

#include "utilities/make_sparse_matrix.h"
#include "utilities/visitor.h"

#include "MatlabDataArray.hpp"

#include <complex>

namespace Moment::mex {

    struct DenseCellBasisVisitor {
    private:
        matlab::engine::MATLABEngine &engine;
        const MatrixProperties &imp;

    public:
        using return_type = std::pair<matlab::data::CellArray, matlab::data::CellArray>;

        DenseCellBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                              const MatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /* Symbolic matrix input -> dense output */
        return_type operator_matrix(const SymbolicMatrix& matrix) {
            const auto& symbols = matrix.Symbols;
            const bool symmetric = this->imp.is_hermitian();
            auto output = create_empty_basis(symbols);

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        matlab::data::TypedArrayRef<double> re_mat = output.first[re_id];
                        re_mat[index_i][index_j] = elem.factor;
                        if (symmetric && (index_i != index_j)) {
                            re_mat[index_j][index_i] = elem.factor;
                        }
                    }

                    if (this->imp.is_complex() && (im_id>=0)) {
                        matlab::data::TypedArrayRef<std::complex<double>> im_mat = output.second[im_id];
                        im_mat[index_i][index_j] = std::complex<double>{0.0,
                                                                        (elem.conjugated ? -1.0 : 1.0) * elem.factor};
                        if (symmetric && (index_i != index_j)) {
                            im_mat[index_j][index_i] = std::complex<double>{
                                    0.0, (elem.conjugated ? 1.0 : -1.0) * elem.factor};
                        }
                    }
                }
            }

            return output;
        }

    private:
        return_type create_empty_basis(const SymbolTable& table) {
            return create_empty_basis(table.RealSymbolIds().size(), table.ImaginarySymbolIds().size(),
                                      this->imp.Dimension());
        }

        static return_type create_empty_basis(const size_t real_elems, const size_t im_elems, const size_t dimension) {
            matlab::data::ArrayFactory factory{};

            matlab::data::ArrayDimensions re_ad{real_elems > 0 ? 1U : 0U, real_elems};
            matlab::data::ArrayDimensions im_ad{im_elems > 0 ? 1U : 0U, im_elems};

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

    struct SparseCellBasisVisitor {
    public:
        using return_type = std::pair<matlab::data::CellArray, matlab::data::CellArray>;

    private:
        matlab::engine::MATLABEngine &engine;
        const MatrixProperties &imp;

        struct sparse_basis_re_frame {
            std::vector<size_t> index_i{};
            std::vector<size_t> index_j{};
            std::vector<double> values{};

            void push_back(size_t i, size_t j, double value) {
                index_i.emplace_back(i);
                index_j.emplace_back(j);
                values.emplace_back(value);
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
            }
        };

    public:
        SparseCellBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                               const MatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /** Moment matrix input -> sparse output */
        return_type operator_matrix(const SymbolicMatrix& matrix) {
            const auto& symbols = matrix.Symbols;

            std::vector<sparse_basis_re_frame> real_basis_frame(symbols.RealSymbolIds().size());
            std::vector<sparse_basis_im_frame> im_basis_frame(symbols.ImaginarySymbolIds().size());
            const bool symmetric = this->imp.is_hermitian();

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        assert(re_id < real_basis_frame.size());
                        real_basis_frame[re_id].push_back(index_i, index_j, elem.factor);
                        if (symmetric && (index_i != index_j)) {
                            real_basis_frame[re_id].push_back(index_j, index_i, elem.factor);
                        }
                    }

                    if (im_id>=0) {
                        assert(im_id < im_basis_frame.size());
                        im_basis_frame[im_id].push_back(index_i, index_j, std::complex<double>{
                                0.0, (elem.conjugated ? -1.0 : 1.0) * elem.factor});
                        if (symmetric && (index_i != index_j)) {
                            im_basis_frame[im_id].push_back(index_j, index_i, std::complex<double>{
                                    0.0, (elem.conjugated ? 1.0 : -1.0) * elem.factor});
                        }
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

    struct DenseMonolithBasisVisitor {
    private:
        matlab::engine::MATLABEngine &engine;
        const MatrixProperties &imp;
    public:
        using return_type = std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>;

        DenseMonolithBasisVisitor(matlab::engine::MATLABEngine &engineRef,
                                  const MatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /** Symbolic matrix input -> dense monolithic output */
        return_type operator_matrix(const SymbolicMatrix& matrix) {
            const auto& symbols = matrix.Symbols;
            const bool symmetric = this->imp.is_hermitian();
            auto output = create_empty_basis(symbols);

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id>=0) {
                        output.first[re_id][flatten_index(index_i, index_j)] = elem.factor;
                        if (symmetric && (index_i != index_j)) {
                            output.first[re_id][flatten_index(index_j, index_i)] = elem.factor;
                        }
                    }

                    if ((this->imp.is_complex()) && (im_id>=0)) {

                        output.second[im_id][flatten_index(index_i, index_j)]
                            = std::complex<double>{0.0, (elem.conjugated ? -1.0 : 1.0) * elem.factor};
                        if (symmetric && (index_i != index_j)) {
                            output.second[im_id][flatten_index(index_j, index_i)]
                                = std::complex<double>{0.0, (elem.conjugated ? 1.0 : -1.0) * elem.factor};
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

    struct SparseMonolithBasisVisitor {
    public:
        using return_type = std::pair<matlab::data::SparseArray<double>,
                matlab::data::SparseArray<std::complex<double>>>;

    private:
        matlab::engine::MATLABEngine &engine;
        const MatrixProperties &imp;

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
                                   const MatrixProperties &matrix_properties)
                : engine(engineRef), imp(matrix_properties) {}

        /** SymbolicMatrix input -> sparse monolithic output */
        return_type operator_matrix(const SymbolicMatrix& matrix) {
            const auto& symbols = matrix.Symbols;
            const bool hasImBasis = this->imp.is_complex();
            const bool symmetric = this->imp.is_hermitian();

            monolith_re_frame real_basis_frame{};
            monolith_im_frame im_basis_frame{};

            for (size_t index_i = 0; index_i < this->imp.Dimension(); ++index_i) {
                for (size_t index_j = symmetric ? index_i : 0 ; index_j < this->imp.Dimension(); ++index_j) {
                    const auto& elem = matrix.SymbolMatrix[index_i][index_j];
                    auto [re_id, im_id] = symbols[elem.id].basis_key();

                    if (re_id >= 0) {
                        real_basis_frame.push_back(re_id, flatten_index(index_i, index_j), elem.factor);

                        if (symmetric && (index_i != index_j)) {
                            real_basis_frame.push_back(re_id, flatten_index(index_j, index_i), elem.factor);
                        }
                    }

                    if (hasImBasis && (im_id>=0)) {
                        im_basis_frame.push_back(im_id, flatten_index(index_i, index_j),
                                                 std::complex<double>{0.0, (elem.conjugated ? -1.0 : 1.0) * elem.factor});
                        if (symmetric && (index_i != index_j)) {
                            im_basis_frame.push_back(im_id, flatten_index(index_j, index_i),
                                                     std::complex<double>{0.0, (elem.conjugated ? 1.0 : -1.0) * elem.factor});
                        }
                    }
                }
            }

            return this->construct_basis(
                    symbols.RealSymbolIds().size(), symbols.ImaginarySymbolIds().size(),
                    real_basis_frame, im_basis_frame);
        }

    private:
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

    std::pair<matlab::data::CellArray, matlab::data::CellArray>
    export_dense_cell_basis(matlab::engine::MATLABEngine &engine, const SymbolicMatrix& mm) {
        DenseCellBasisVisitor dcbv{engine, mm.SMP()};
        return dcbv.operator_matrix(mm);
    }

    std::pair<matlab::data::CellArray, matlab::data::CellArray>
    export_sparse_cell_basis(matlab::engine::MATLABEngine &engine,
                                       const SymbolicMatrix& mm) {
        SparseCellBasisVisitor scbv{engine, mm.SMP()};
        return scbv.operator_matrix(mm);
    }

    std::pair<matlab::data::TypedArray<double>, matlab::data::TypedArray<std::complex<double>>>
    export_dense_monolith_basis(matlab::engine::MATLABEngine &engine,
                                          const SymbolicMatrix& mm) {
        DenseMonolithBasisVisitor dmbv{engine, mm.SMP()};
        return dmbv.operator_matrix(mm);
    }

    std::pair<matlab::data::SparseArray<double>, matlab::data::SparseArray<std::complex<double>>>
    export_sparse_monolith_basis(matlab::engine::MATLABEngine &engine, const SymbolicMatrix& mm) {
        SparseMonolithBasisVisitor smbv{engine, mm.SMP()};
        return smbv.operator_matrix(mm);
    }



}