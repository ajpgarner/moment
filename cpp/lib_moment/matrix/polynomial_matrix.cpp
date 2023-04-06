/**
 * polynomial_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_matrix.h"

#include "symbolic/symbol_table.h"

namespace Moment {

    PolynomialMatrix::PolynomialMatrix(const Context& context, SymbolTable& symbols,
                     std::unique_ptr<SquareMatrix<SymbolCombo>> symbolMatrix)
             : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0}, SymbolMatrix{*this},
               sym_exp_matrix{std::move(symbolMatrix)} {
        if (!sym_exp_matrix) {
            throw std::runtime_error{"Symbol matrix pointer passed to PolynomialMatrix constructor was nullptr."};
        }

        // Find included symbols
        std::set<symbol_name_t> included_symbols;
        const size_t max_symbol_id = symbols.size();
        for (const auto& poly : *sym_exp_matrix) {
            for (const auto& term : poly) {
                included_symbols.emplace(term.id);
            }
        }

        // Create symbol matrix properties
        this->mat_prop = std::make_unique<MatrixProperties>(*this, this->symbol_table, std::move(included_symbols));


    }


    std::pair<Matrix::MatrixBasis::dense_real_storage_t, Matrix::MatrixBasis::dense_complex_storage_t>
    PolynomialMatrix::create_dense_basis() const {
        throw std::runtime_error{"PolynomialMatrix::create_dense_basis() not implemented."};

         std::pair<MatrixBasis::dense_real_storage_t, MatrixBasis::dense_complex_storage_t> output;
         return output;
    }

    std::pair<Matrix::MatrixBasis::sparse_real_storage_t, Matrix::MatrixBasis::sparse_complex_storage_t>
    PolynomialMatrix::create_sparse_basis() const {
        throw std::runtime_error{"PolynomialMatrix::create_sparse_basis() not implemented."};

        std::pair<MatrixBasis::sparse_real_storage_t, MatrixBasis::sparse_complex_storage_t> output;
        return output;
    }
}