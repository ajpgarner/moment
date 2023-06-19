/**
 * polynomial_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * @see polynomial_matrix_basis.cpp for basis function definitions.
 */

#include "polynomial_matrix.h"

#include "symbolic/polynomial_to_basis.h"
#include "symbolic/polynomial_to_basis_mask.h"
#include "symbolic/symbol_table.h"

namespace Moment {

    namespace {
        bool test_hermicity(const SymbolTable &table, const SquareMatrix<Polynomial> &matrix, double tolerance) {

            for (size_t row = 0; row < matrix.dimension; ++row) {
                if (!matrix[row][row].is_hermitian(table, tolerance)) {
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

    PolynomialMatrix::PolynomialMatrix(const Context& context, SymbolTable& symbols, const double zero_tolerance,
                     std::unique_ptr<SquareMatrix<Polynomial>> symbolMatrix)
             : Matrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0}, SymbolMatrix{*this},
               sym_exp_matrix{std::move(symbolMatrix)} {
        if (!sym_exp_matrix) {
            throw std::runtime_error{"Symbol matrix pointer passed to PolynomialMatrix constructor was nullptr."};
        }

        // Matrix properties
        this->hermitian = test_hermicity(symbols, *sym_exp_matrix, 1.0);
        this->description = "Polynomial Symbolic Matrix";

        // Included symbols and basis elements
        this->identify_symbols_and_basis_indices(zero_tolerance);
    }

    /**
     * Force renumbering of matrix bases keys
     */
    void PolynomialMatrix::renumerate_bases(const SymbolTable& symbols, double zero_tolerance) {
        for (auto& polynomial : *this->sym_exp_matrix) {
            polynomial.fix_cc_in_place(symbols, true, zero_tolerance);
        }

        this->identify_symbols_and_basis_indices(zero_tolerance);
    }

    void PolynomialMatrix::identify_symbols_and_basis_indices(double zero_tolerance) {
        // Find and canonicalize included symbols
        const size_t max_symbol_id = symbols.size();
        this->complex_coefficients = false;
        this->included_symbols.clear();

        PolynomialToBasisMask ptm{this->symbols, zero_tolerance};

        auto [real_mask, im_mask] = ptm.empty_mask();

        for (auto& poly : *sym_exp_matrix) {
            // Get raw symbols and coefficients
            for (auto &monomial: poly) {
                assert(monomial.id < max_symbol_id);
                this->included_symbols.emplace(monomial.id);
                if (!this->complex_coefficients && monomial.complex_factor()) { // <- first clause, avoid unnecessary tests
                    this->complex_coefficients = true;
                }
            }
            ptm.set_bits(real_mask, im_mask, poly);
        }

        this->real_basis_elements = real_mask.to_set();
        this->imaginary_basis_elements = im_mask.to_set();

        // Now make basis key [could include some basis elements that do not appear due to terms like X + X*].
        this->basis_key.clear();
        for (const auto symbol_id : this->included_symbols) {
            auto &symbol_info = this->symbols[symbol_id];
            auto [re_key, im_key] = symbol_info.basis_key();
            this->basis_key.emplace_hint(this->basis_key.end(),
                                         std::make_pair(symbol_id, std::make_pair(re_key, im_key)));
        }

        this->complex_basis = !this->imaginary_basis_elements.empty();
    }


}