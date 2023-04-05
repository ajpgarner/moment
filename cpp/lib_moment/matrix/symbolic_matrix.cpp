/**
 * symbolic_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbolic_matrix.h"

#include <stdexcept>

namespace Moment {


    SymbolicMatrix::SymbolicMatrix(const Context& context, SymbolTable& symbols,
                                   std::unique_ptr<SquareMatrix<SymbolExpression>> symbolMatrix)
        : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0},
            SymbolMatrix{*this}, sym_exp_matrix{std::move(symbolMatrix)}
        {
            if (!sym_exp_matrix) {
                throw std::runtime_error{"Symbol pointer passed to SymbolicMatrix was nullptr."};
            }

            // Find included symbols
            std::set<symbol_name_t> included_symbols;
            const size_t max_symbol_id = symbols.size();
            for (const auto& x : *sym_exp_matrix) {
                assert(x.id < max_symbol_id);
                included_symbols.emplace(x.id);
            }

            // Create symbol matrix properties
            this->mat_prop = std::make_unique<MatrixProperties>(*this, this->symbol_table, std::move(included_symbols));

    }

    void SymbolicMatrix::renumerate_bases(const SymbolTable &symbols) {
        for (auto& symbol : *this->sym_exp_matrix) {
            // Make conjugation status canonical:~
            if (symbol.conjugated) {
                const auto& ref_symbol = symbols[symbol.id];
                if (ref_symbol.is_hermitian()) {
                    symbol.conjugated = false;
                } else if (ref_symbol.is_antihermitian()) {
                    symbol.conjugated = false;
                    symbol.factor *= -1.0;
                }
            }
        }

        this->mat_prop->rebuild_keys(symbols);
    }

    SymbolicMatrix::~SymbolicMatrix() noexcept = default;

    std::pair<dense_basis_storage_t, dense_basis_storage_t>
    SymbolicMatrix::create_dense_basis() const {
        std::pair<dense_basis_storage_t, dense_basis_storage_t> output;
        auto& real = output.first;
        auto& im = output.second;

        auto dim = static_cast<dense_basis_elem_t::Index>(this->dimension);

        real.assign(this->symbol_table.RealSymbolIds().size(),
                            dense_basis_elem_t::Zero(dim, dim));
        im.assign(this->symbol_table.ImaginarySymbolIds().size(),
                            dense_basis_elem_t::Zero(dim, dim));

        const bool symmetric = this->SMP().is_hermitian();
        const bool complex = this->SMP().is_complex();

        for (int row_index = 0; row_index < this->dimension; ++row_index) {
            for (int col_index = symmetric ? row_index : 0; col_index < this->dimension; ++col_index) {
                const auto& elem = this->SymbolMatrix[row_index][col_index];
                assert(elem.id < this->Symbols.size());
                auto [re_id, im_id] = this->Symbols[elem.id].basis_key();

                if (re_id>=0) {
                    assert(re_id < real.size());
                    real[re_id](row_index, col_index) = elem.factor;
                    if (symmetric && (row_index != col_index)) {
                        real[re_id](col_index, row_index) = elem.factor;
                    }
                }

                if (complex && (im_id>=0)) {
                    assert(im_id < im.size());

                    im[im_id](row_index, col_index) = (elem.conjugated ? -1.0 : 1.0) * elem.factor;
                    if (symmetric && (row_index != col_index)) {
                        im[im_id](col_index, row_index) = (elem.conjugated ? 1.0 : -1.0) * elem.factor;
                    }

                }
            }
        }
        return output;
    }


    std::pair<sparse_basis_storage_t, sparse_basis_storage_t>
    SymbolicMatrix::create_sparse_basis() const {
        // Get matrix properties
        const auto dim = static_cast<sparse_basis_elem_t::Index>(this->dimension);
        const bool symmetric = this->SMP().is_hermitian();
        const bool complex = this->SMP().is_complex();

        // Prepare triplets
        using trip_t = Eigen::Triplet<sparse_basis_elem_t::value_type>;
        std::vector<std::vector<trip_t>> real_frame(this->Symbols.RealSymbolIds().size());
        std::vector<std::vector<trip_t>> im_frame(this->Symbols.ImaginarySymbolIds().size());

        for (int row_index = 0; row_index < this->dimension; ++row_index) {
            for (int col_index = symmetric ? row_index : 0; col_index < this->dimension; ++col_index) {
                const auto& elem = this->SymbolMatrix[row_index][col_index];
                assert(elem.id < this->Symbols.size());
                auto [re_id, im_id] = this->Symbols[elem.id].basis_key();

                if (re_id>=0) {
                    assert(re_id < real_frame.size());
                    real_frame[re_id].emplace_back(row_index, col_index, elem.factor);
                    if (symmetric && (row_index != col_index)) {
                        real_frame[re_id].emplace_back(col_index, row_index, elem.factor);
                    }
                }

                if (complex && (im_id>=0)) {
                    assert(im_id < im_frame.size());
                    im_frame[im_id].emplace_back(row_index, col_index,
                                                 (elem.conjugated ? -1.0 : 1.0) * elem.factor);
                    if (symmetric && (row_index != col_index)) {
                        im_frame[im_id].emplace_back(col_index, row_index,
                                                     (elem.conjugated ? -1.0 : 1.0) * elem.factor);
                    }
                }
            }
        }

        // Now, build sparse matrices
        std::pair<sparse_basis_storage_t, sparse_basis_storage_t> output;
        auto& real = output.first;
        real.assign(real_frame.size(), sparse_basis_elem_t(dim, dim));
        for (size_t re_index = 0; re_index < real_frame.size(); ++re_index) {
            real[re_index].setFromTriplets(real_frame[re_index].cbegin(), real_frame[re_index].cend());
        }

        auto& im = output.second;
        im.assign(im_frame.size(), sparse_basis_elem_t(dim, dim));
        for (size_t im_index = 0; im_index < im_frame.size(); ++im_index) {
            im[im_index].setFromTriplets(im_frame[im_index].cbegin(), im_frame[im_index].cend());
        }

        // Return
        return output;
    }

}