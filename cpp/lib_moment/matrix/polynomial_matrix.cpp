/**
 * polynomial_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_matrix.h"

#include "symbolic/symbol_table.h"

namespace Moment {

    namespace {
        bool test_hermicity(const SymbolTable &table, const SquareMatrix<Polynomial> &matrix) {

            for (size_t row = 0; row < matrix.dimension; ++row) {
                if (!matrix[row][row].is_hermitian(table)) {
                    return false;
                }

                for (size_t col = row + 1; col < matrix.dimension; ++col) {
                    const auto &upper = matrix[row][col];
                    const auto &lower = matrix[col][row];
                    if (!upper.is_conjugate(table, lower)) {
                        return false;
                    }
                }
            }

            return true;
        }

    }

    PolynomialMatrix::PolynomialMatrix(const Context& context, SymbolTable& symbols,
                     std::unique_ptr<SquareMatrix<Polynomial>> symbolMatrix)
             : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0}, SymbolMatrix{*this},
               sym_exp_matrix{std::move(symbolMatrix)}, real_prefactors{true} {
        if (!sym_exp_matrix) {
            throw std::runtime_error{"Symbol matrix pointer passed to PolynomialMatrix constructor was nullptr."};
        }

        // Find included symbols
        std::set<symbol_name_t> included_symbols;
        const size_t max_symbol_id = symbols.size();
        for (const auto& poly : *sym_exp_matrix) {
            for (const auto& term : poly) {
                included_symbols.emplace(term.id);
                if (this->real_prefactors && term.complex_factor()) { // <- first clause, avoid unnecessary tests
                    this->real_prefactors = false;
                }
            }
        }

        // Test for Hermiticity
        const bool is_hermitian = test_hermicity(symbols, *sym_exp_matrix);

        // Create symbol matrix properties
        this->mat_prop = std::make_unique<MatrixProperties>(*this, this->symbol_table, std::move(included_symbols),
                                                            "Polynomial Symbolic Matrix", is_hermitian);

    }

    /**
     * Force renumbering of matrix bases keys
     */
    void PolynomialMatrix::renumerate_bases(const SymbolTable& symbols) {
        // TODO: Support renumbering for polynomial matrix
        throw std::runtime_error{"PolynomialMatrix::renumerate_bases not implemented."};
    }
}