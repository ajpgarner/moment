/**
 * symbolic_matrix.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "symbolic_matrix.h"
#include "composite_matrix.h"

#include "matrix_system/matrix_system.h"
#include "operator_matrix/operator_matrix.h"

#include <iostream>

namespace Moment {

    SymbolicMatrix::SymbolicMatrix(const Context& context, SymbolTable& symbols, size_t dimension )
        : context{context}, symbols{symbols}, symbol_table{symbols}, dimension{dimension}, Basis{*this} {

        if (debug_mode) {
            this->description = "Abstract Matrix";
        }
    }

    SymbolicMatrix::~SymbolicMatrix() noexcept = default;

    bool SymbolicMatrix::has_aliased_operator_matrix() const noexcept {
        // If we have it, simply return yes
        if (this->aliased_op_mat) {
            return true;
        }

        // Otherwise, in unaliasing scenarios, return whether we have the raw operator matrix.
        if (!this->context.can_have_aliases()) {
            return this->unaliased_op_mat != nullptr;
        } else {
            return false;
        }
    }

    const OperatorMatrix& SymbolicMatrix::unaliased_operator_matrix() const {
        if (!this->unaliased_op_mat) {
            throw errors::missing_component{"No operator matrix defined for this matrix."};
        }
        return *this->unaliased_op_mat;
    }

    const OperatorMatrix& SymbolicMatrix::aliased_operator_matrix() const {
        // If we have it, simplify return it
        if (this->aliased_op_mat) {
            return *this->aliased_op_mat;
        }

        // If we don't have it, should we have it?
        if (this->context.can_have_aliases()) {
            throw errors::missing_component{"No aliased matrix was defined for this matrix."};
        }

        // Otherwise, we can default to unaliased op matrix.
        return this->unaliased_operator_matrix();
    }

    void SymbolicMatrix::throw_error_if_cannot_multiply() const {
        // Get operator matrix
        if (!this->has_unaliased_operator_matrix()) {
            throw errors::cannot_multiply_exception{"MonomialMatrix cannot multiply if OperatorMatrix is not present."};
        }
    }


    std::unique_ptr<SymbolicMatrix> SymbolicMatrix::clone(Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_clone_exception{"Generic SymbolicMatrix cannot be cloned."};
    }

    void SymbolicMatrix::copy_properties_onto_clone(SymbolicMatrix& clone) const {
        // Basic properties must already match
        assert(&clone.context == &this->context);
        assert(&clone.symbol_table == &this->symbol_table);

        // Copy resolved matrix properties
        clone.dimension = this->dimension;
        clone.hermitian = this->hermitian;
        clone.complex_coefficients = this->complex_coefficients;
        clone.complex_basis = this->complex_basis;
        clone.description = this->description;
        clone.included_symbols = this->included_symbols;
        clone.real_basis_elements = this->real_basis_elements;
        clone.imaginary_basis_elements = this->imaginary_basis_elements;
        clone.basis_key = this->basis_key;
    }


    std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::pre_multiply(const OperatorSequence& sequence, std::complex<double> weight,
                                 const PolynomialFactory& poly_factory,
                                 SymbolTable& symbol_registry,
                                 const Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_multiply_exception{
            "Pre-multiplication by operator sequence not defined for generic SymbolicMatrix."
        };
    }

    std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::post_multiply(const OperatorSequence& sequence, std::complex<double> weight,
                                  const PolynomialFactory& poly_factory,
                                  SymbolTable& symbol_registry,
                                  const Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_multiply_exception{
                "Pre-multiplication by operator sequence not defined for generic SymbolicMatrix."
        };
    }


    std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::pre_multiply(const RawPolynomial& lhs, const PolynomialFactory& poly_factory,
                                 SymbolTable& symbol_registry,
                                 const Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_multiply_exception{
                "Pre-multiplication by raw polynomial not defined for generic SymbolicMatrix."
        };
    }

    std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::post_multiply(const RawPolynomial& rhs,
                                  const PolynomialFactory& poly_factory, SymbolTable& symbol_registry,
                                  const Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_multiply_exception{
                "Pre-multiplication by raw polynomial not defined for generic SymbolicMatrix."
        };
    }



    namespace {
        template<bool premultiply>
        inline std::unique_ptr<SymbolicMatrix>
        do_monomial_multiply(const Monomial& mono, const SymbolicMatrix& matrix,
                             const PolynomialFactory& poly_factory, SymbolTable& symbol_registry,
                             Multithreading::MultiThreadPolicy mt_policy) {

            // Special case: zero:
            if (mono.id == 0) {
                return MonomialMatrix::zero_matrix(matrix.context, symbol_registry, matrix.Dimension());
            }

            // Special case: identity
            if (mono.id == 1) {
                if (approximately_equal(mono.factor, 1.0, poly_factory.zero_tolerance)) {
                    return matrix.clone(mt_policy);
                }
            }

            // Check matrix can be multiplied
            matrix.throw_error_if_cannot_multiply();

            // Resolve monomial into operator sequence using symbol table
            assert(mono.id >= 0 && mono.id < symbol_registry.size());
            assert(symbol_registry[mono.id].has_sequence());
            const auto &op_sequence = mono.conjugated ? symbol_registry[mono.id].sequence()
                                                      : symbol_registry[mono.id].sequence_conj();

            // Invoke operator sequence multiplication
            if constexpr (premultiply) {
                return matrix.pre_multiply(op_sequence, mono.factor,  poly_factory, symbol_registry, mt_policy);
            } else {
                return matrix.post_multiply(op_sequence, mono.factor, poly_factory, symbol_registry, mt_policy);
            }
        }
    }

    [[nodiscard]] std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::pre_multiply(const Monomial& lhs, const PolynomialFactory& poly_factory,
                                 SymbolTable& mutating_symbol_table,
                                 Multithreading::MultiThreadPolicy policy) const {
        // The reason we don't just read symbol_table out from the class, but instead pass as a reference argument, is
        // that a const method causing mutating behaviour in a referenced object is deeply upsetting, even if valid C++.
        assert(&this->symbol_table == &mutating_symbol_table);
        return do_monomial_multiply<true>(lhs, *this, poly_factory, mutating_symbol_table, policy);
    }

    [[nodiscard]] std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::post_multiply(const Monomial& rhs,
                                  const PolynomialFactory& poly_factory, SymbolTable& mutating_symbol_table,
                                  Multithreading::MultiThreadPolicy policy) const {
        assert(&this->symbol_table == &mutating_symbol_table);
        return do_monomial_multiply<false>(rhs, *this, poly_factory, mutating_symbol_table, policy);
    }

    namespace {
        template<bool premultiply>
        inline std::unique_ptr<SymbolicMatrix>
        do_polynomial_multiply(const Polynomial& poly, const SymbolicMatrix& matrix,
                               const PolynomialFactory& factory, SymbolTable& symbol_table,
                               Multithreading::MultiThreadPolicy mt_policy) {

            // Special case, zero:
            if (poly.empty()) {
                return MonomialMatrix::zero_matrix(matrix.context, symbol_table, matrix.Dimension());
            }

            // Special case, monomial (including scalars):
            if (poly.is_monomial()) {
                assert(!poly.empty()); // <- should be handled by special case zero already
                return do_monomial_multiply<premultiply>(poly.back(), matrix, factory, symbol_table, mt_policy);
            }

            // Resolve polynomial into operator sequences
            RawPolynomial raw_poly{poly, symbol_table};

            // Invoke general case:
            if constexpr (premultiply) {
                return matrix.pre_multiply(raw_poly, factory, symbol_table, mt_policy);
            } else {
                return matrix.post_multiply(raw_poly, factory, symbol_table, mt_policy);
            }
        }
    }

    [[nodiscard]] std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::pre_multiply(const Polynomial& lhs,
                                 const PolynomialFactory& poly_factory, SymbolTable& mutating_symbol_table,
                                 Multithreading::MultiThreadPolicy policy) const {
        assert(&this->symbol_table == &mutating_symbol_table);
        return do_polynomial_multiply<true>(lhs, *this, poly_factory, mutating_symbol_table, policy);
    }

    [[nodiscard]] std::unique_ptr<SymbolicMatrix>
    SymbolicMatrix::post_multiply(const Polynomial& rhs,
                                  const PolynomialFactory& poly_factory, SymbolTable& mutating_symbol_table,
                                  Multithreading::MultiThreadPolicy policy) const {
        assert(&this->symbol_table == &mutating_symbol_table);
        return do_polynomial_multiply<false>(rhs, *this, poly_factory, mutating_symbol_table, policy);
    }

    std::unique_ptr<PolynomialMatrix>
    SymbolicMatrix::add(const SymbolicMatrix& rhs, const PolynomialFactory& poly_factory,
                        Multithreading::MultiThreadPolicy policy) const {

        // First, check we can add...
        if (this->dimension != rhs.Dimension()) {
            throw errors::cannot_add_exception{"Cannot add matrices with mismatched dimensions."};
        }

        // Register as constituents, and make a composite polynomial matrix
        CompositeMatrix::ConstituentInfo constituent_data;
        constituent_data.matrix_dimension = this->dimension;
        constituent_data.elements.emplace_back(this, std::complex<double>(1.0, 0.0));
        constituent_data.elements.emplace_back(&rhs, std::complex<double>(1.0, 0.0));

        return std::make_unique<CompositeMatrix>(this->context, this->symbol_table,
                                                 poly_factory, std::move(constituent_data));


    }

    std::unique_ptr<PolynomialMatrix> SymbolicMatrix::add(const Monomial& rhs,
                                                          const PolynomialFactory& poly_factory,
                                                          Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_add_exception{"Addition not defined for generic SymbolicMatrix."};
    }


    std::unique_ptr<PolynomialMatrix> SymbolicMatrix::add(const Polynomial& rhs,
                                                          const PolynomialFactory& poly_factory,
                                                          Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_add_exception{"Addition not defined for generic SymbolicMatrix."};
    }


    std::ostream& operator<<(std::ostream& os, const SymbolicMatrix& mp) {
        os << mp.dimension << "x" << mp.dimension << " ";
        if (mp.complex_basis) {
            if (mp.hermitian) {
                os << "Hermitian matrix";
            } else {
                os << "Complex matrix";
            }
        } else {
            if (mp.hermitian) {
                os << "Symmetric matrix";
            } else {
                os << "Real matrix";
            }
        }
        const auto num_us = mp.included_symbols.size();
        os << " with "
           << num_us << " unique " << (num_us != 1 ? "symbols" : "symbol");
        const auto num_re = mp.real_basis_elements.size();
        if (num_re > 0) {
            os << ", " << num_re << " real";
        }
        const auto num_im = mp.imaginary_basis_elements.size();
        if (num_im > 0) {
            os << ", " << num_im << " imaginary";
        }
        os << ".";
        return os;
    }


}