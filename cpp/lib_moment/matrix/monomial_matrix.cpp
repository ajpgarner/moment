/**
 * monomial_matrix.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 *
 * @see monomial_matrix_basis.cpp for basis function definitions.
 */
#include "monomial_matrix.h"
#include "polynomial_matrix.h"
#include "operator_matrix/operator_matrix.h"

#include "dictionary/raw_polynomial.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"

#include "utilities/float_utils.h"

#include <stdexcept>


namespace Moment {
    MonomialMatrix::MonomialMatrix(const Context& context, SymbolTable& symbols, const double zero_tolerance,
                                   std::unique_ptr<SquareMatrix<Monomial>> symbolMatrix,
                                   const bool constructed_as_hermitian, std::complex<double> factor)
        : SymbolicMatrix{context, symbols, symbolMatrix ? symbolMatrix->dimension : 0},
          SymbolMatrix{*this}, sym_exp_matrix{std::move(symbolMatrix)}, global_prefactor{factor}
        {
            // Sanity check
            if (!sym_exp_matrix) {
                throw std::runtime_error{"Symbol pointer passed to MonomialMatrix constructor was nullptr."};
            }

            // Count symbols
            this->MonomialMatrix::renumerate_bases(symbols, zero_tolerance);

            // Set matrix properties
            this->description = "Monomial Symbolic Matrix";
            this->hermitian = constructed_as_hermitian;
    }


    MonomialMatrix::MonomialMatrix(SymbolTable& symbols,
                                   std::unique_ptr<OperatorMatrix> unaliased_mat_ptr,
                                   std::unique_ptr<OperatorMatrix> aliased_mat_ptr,
                                   std::unique_ptr<SquareMatrix<Monomial>> sym_mat_ptr,
                                   std::complex<double> prefactor)
            : MonomialMatrix{unaliased_mat_ptr->context, symbols, 1.0, std::move(sym_mat_ptr),
                             (aliased_mat_ptr != nullptr) ? aliased_mat_ptr->is_hermitian()
                                                          : unaliased_mat_ptr->is_hermitian(),
                             prefactor} {

        // Sanity check
        if (!sym_exp_matrix) {
            throw std::runtime_error{"Symbol pointer passed to MonomialMatrix constructor was nullptr."};
        }

        // Register operator matrix with this monomial matrix
        this->unaliased_op_mat = std::move(unaliased_mat_ptr);
        this->aliased_op_mat = std::move(aliased_mat_ptr);

        // Preferably take properties from aliased matrix
        if (this->aliased_op_mat) {
            this->aliased_op_mat->set_properties(*this);
        } else if (this->unaliased_op_mat) { // Otherwise, properties from unaliased matrix
            this->unaliased_op_mat->set_properties(*this);
        }
    }

    MonomialMatrix::~MonomialMatrix() noexcept = default;

    void MonomialMatrix::renumerate_bases(const SymbolTable &symbols, double zero_tolerance) {
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
            // If zero, replace with canonical zero.
            if (approximately_zero(symbol.factor, zero_tolerance)) {
                symbol.id = 0;
                symbol.conjugated = false;
                symbol.factor = 0;
            }
        }

        this->identify_symbols_and_basis_indices();
    }

    void MonomialMatrix::identify_symbols_and_basis_indices() {
        // Find and canonicalize included symbols
        const size_t max_symbol_id = symbols.size();
        this->complex_coefficients = false;
        this->included_symbols.clear();
        for (auto& x : *sym_exp_matrix) {
            assert(x.id < max_symbol_id);
            this->included_symbols.emplace(x.id);
            if (!this->complex_coefficients && x.complex_factor()) { // <- first clause, avoid unnecessary tests
                this->complex_coefficients = true;
            }
        }

        // All included symbols:~
        this->real_basis_elements.clear();
        this->imaginary_basis_elements.clear();
        this->basis_key.clear();
        for (const auto symbol_id : this->included_symbols) {
            auto &symbol_info = this->symbols[symbol_id];
            auto [re_key, im_key] = symbol_info.basis_key();
            if (re_key >= 0) {
                this->real_basis_elements.emplace(re_key);
            }
            if (im_key >= 0) {
                this->imaginary_basis_elements.emplace(im_key);
            }
            this->basis_key.emplace_hint(this->basis_key.end(),
                                         std::make_pair(symbol_id, std::make_pair(re_key, im_key)));
        }

        this->complex_basis = !this->imaginary_basis_elements.empty();
    };

    std::unique_ptr<MonomialMatrix>
    MonomialMatrix::zero_matrix(const Context& context, SymbolTable& symbol_table, const size_t dimension) {
        // Symbolic info: all zeros
        auto symbolic_data = std::make_unique<SquareMatrix<Monomial>>(
                dimension, std::vector<Monomial>(dimension * dimension, Monomial{0, 0.0, false})
            );

        // Operator sequence info: all empty operator sequences
        if (context.defines_operators()) {
            auto operator_data = std::make_unique<OperatorMatrix>(context, dimension,
                std::vector<OperatorSequence>(dimension * dimension, OperatorSequence::Zero(context)));

            std::unique_ptr<OperatorMatrix> aliased_matrix;
            if (context.can_have_aliases()) {
                aliased_matrix = std::make_unique<OperatorMatrix>(context, dimension,
                  std::vector<OperatorSequence>(dimension * dimension, OperatorSequence::Zero(context)));
            }


            // Operator sequence info: all zeros
            return std::make_unique<MonomialMatrix>(symbol_table,
                                                    std::move(operator_data),
                                                    std::move(aliased_matrix),
                                                    std::move(symbolic_data));
        }

        // Otherwise, we can construct w/o operator sequences
        return std::make_unique<MonomialMatrix>(context, symbol_table,
                                                1.0, // <- valid, as everything is already zero
                                                std::move(symbolic_data), true);
    }

    std::unique_ptr<SymbolicMatrix> MonomialMatrix::clone(Multithreading::MultiThreadPolicy policy) const {
        // Copy symbol data
        std::vector<Monomial> cloned_symbol_data;
        cloned_symbol_data.reserve(this->dimension * this->dimension);
        std::copy(this->sym_exp_matrix->begin(), this->sym_exp_matrix->end(), std::back_inserter(cloned_symbol_data));
        auto cloned_symbol_matrix =  std::make_unique<SquareMatrix<Monomial>>(this->dimension,
                                                                              std::move(cloned_symbol_data));

        // Make copy of the matrix
        std::unique_ptr<MonomialMatrix> copied_matrix =
                std::make_unique<MonomialMatrix>(symbol_table,
                                                this->has_unaliased_operator_matrix()
                                                    ? this->unaliased_operator_matrix().clone(policy)
                                                    : nullptr,
                                                context.can_have_aliases() && this->has_aliased_operator_matrix()
                                                     ? this->aliased_operator_matrix().clone(policy)
                                                     : nullptr,
                                                 std::move(cloned_symbol_matrix));

        // Copy other matrix properties:
        this->copy_properties_onto_clone(*copied_matrix);
        copied_matrix->global_prefactor = this->global_prefactor;

        return copied_matrix;
    }


}