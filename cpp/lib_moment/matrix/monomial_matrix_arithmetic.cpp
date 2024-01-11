/**
 * monomial_matrix_multiplication.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "monomial_matrix.h"
#include "polynomial_matrix.h"

#include "operator_matrix/operator_matrix.h"

#include "dictionary/operator_sequence.h"
#include "dictionary/raw_polynomial.h"

#include "scenarios/context.h"
#include "symbolic/polynomial_factory.h"
#include "symbolic/symbol_table.h"


namespace Moment {


    std::unique_ptr<PolynomialMatrix>
    MonomialMatrix::add(const SymbolicMatrix& rhs, const PolynomialFactory& poly_factory,
                        Multithreading::MultiThreadPolicy policy) const {
        if (rhs.is_monomial()) {
            return this->add(dynamic_cast<const MonomialMatrix&>(rhs), poly_factory, policy);
        } else {
            const auto& poly_rhs = dynamic_cast<const PolynomialMatrix&>(rhs);
            return poly_rhs.add(*this, poly_factory, policy);
        }
    }

    std::unique_ptr<PolynomialMatrix>
    MonomialMatrix::add(const MonomialMatrix& rhs, const PolynomialFactory& poly_factory,
                        Multithreading::MultiThreadPolicy policy) const {
        if (this->dimension != rhs.dimension) {
            throw errors::cannot_add_exception{"Cannot add matrices with mismatched dimensions."};
        }

        std::array<const MonomialMatrix*, 2> summand_ptrs{this, &rhs};
        return std::make_unique<PolynomialMatrix>(this->context, poly_factory, this->symbol_table, summand_ptrs);
    }

    std::unique_ptr<PolynomialMatrix> MonomialMatrix::add(const Polynomial& rhs, const PolynomialFactory& poly_factory,
                                                          Multithreading::MultiThreadPolicy policy) const {
        throw errors::cannot_add_exception{"MonomialMatrix::add Polynomial RHS not implemented."};
    }



    namespace {
        /** Implementation of monomial multiplication */
        template<bool premultiply>
        std::unique_ptr<MonomialMatrix>
        do_raw_monomial_multiply(const OperatorSequence& op_sequence, std::complex<double> mono_factor,
                                 const MonomialMatrix& matrix,
                                 const PolynomialFactory& poly_factory, SymbolTable& symbol_registry,
                                 const Multithreading::MultiThreadPolicy policy) {

            // Special case, zero operator sequence:
            if (op_sequence.zero()) {
                return MonomialMatrix::zero_matrix(matrix.context, symbol_registry, matrix.Dimension());
            }

            // General case.
            // First, ensure matrix can be multiplied
            matrix.throw_error_if_cannot_multiply();

            // Prefactor multiplication
            const auto new_factor = matrix.global_factor() * mono_factor;

            // Do multiplication
            std::unique_ptr<OperatorMatrix> multiplied_op_ptr;
            if constexpr (premultiply) {
                multiplied_op_ptr = matrix.operator_matrix().pre_multiply(op_sequence, policy);
            } else {
                multiplied_op_ptr = matrix.operator_matrix().post_multiply(op_sequence, policy);
            }

            // Do creation
            return std::make_unique<MonomialMatrix>(symbol_registry, std::move(multiplied_op_ptr), new_factor);
        }
    }

    std::unique_ptr<SymbolicMatrix>
    MonomialMatrix::pre_multiply(const OperatorSequence &lhs, std::complex<double> weight,
                                 const PolynomialFactory& poly_factory, SymbolTable &symbol_table,
                                 Multithreading::MultiThreadPolicy policy) const {
        return do_raw_monomial_multiply<true>(lhs, weight, *this, poly_factory, symbol_table, policy);
    }

    std::unique_ptr<SymbolicMatrix>
    MonomialMatrix::post_multiply(const OperatorSequence &rhs, std::complex<double> weight,
                                  const PolynomialFactory& poly_factory, SymbolTable &symbol_table,
                                  Multithreading::MultiThreadPolicy policy) const {
        return do_raw_monomial_multiply<false>(rhs, weight, *this, poly_factory, symbol_table, policy);
    }

    namespace {
        /** Implementation of polynomial multiplication */
        template<bool premultiply>
        std::unique_ptr<SymbolicMatrix>
        do_raw_polynomial_multiply(const RawPolynomial &poly, const MonomialMatrix &matrix,
                                   const PolynomialFactory& poly_factory, SymbolTable& symbol_registry,
                                   const Multithreading::MultiThreadPolicy policy) {
            // Special case: zero
            if (poly.empty()) {
                return MonomialMatrix::zero_matrix(matrix.context, symbol_registry, matrix.Dimension());
            }

            // Special case: monomial
            if (1 == poly.size()) {
                return do_raw_monomial_multiply<premultiply>(poly[0].sequence, poly[0].weight, matrix,
                                                             poly_factory, symbol_registry, policy);
            }

            // Otherwise, proceed to general case
            matrix.throw_error_if_cannot_multiply();

            // Do multiplication of operator matrices
            const size_t poly_size = poly.size();
            auto& op_mat = matrix.operator_matrix();
            std::vector<std::unique_ptr<OperatorMatrix>> multiplied_op_mats;
            if constexpr (premultiply) {
                multiplied_op_mats = op_mat.pre_multiply(poly, policy);
            } else {
                multiplied_op_mats = op_mat.post_multiply(poly, policy);
            }
            assert(multiplied_op_mats.size() == poly_size);

            // Calculate symbols [at this stage, we will consider weights]
            std::vector<std::unique_ptr<MonomialMatrix>> symbolized_op_mats;
            symbolized_op_mats.reserve(poly_size);
            std::vector<const MonomialMatrix*> raw_ptrs;
            raw_ptrs.reserve(poly_size);
            for (size_t n = 0; n < poly_size; ++n) {
                symbolized_op_mats.emplace_back(
                        std::make_unique<MonomialMatrix>(symbol_registry, std::move(multiplied_op_mats[n]),
                                                         matrix.global_factor() * poly[n].weight));
                raw_ptrs.emplace_back(symbolized_op_mats.back().get());
            }

            // Combine into Polynomial matrix
            return std::make_unique<PolynomialMatrix>(matrix.context, poly_factory, symbol_registry, raw_ptrs);
        }
    }


    std::unique_ptr<SymbolicMatrix>
    MonomialMatrix::pre_multiply(const RawPolynomial &lhs,
                                 const PolynomialFactory& poly_factory, SymbolTable &symbol_table,
                                 Multithreading::MultiThreadPolicy policy) const {
        return do_raw_polynomial_multiply<true>(lhs, *this, poly_factory, symbol_table, policy);
    }

    std::unique_ptr<SymbolicMatrix>
    MonomialMatrix::post_multiply(const RawPolynomial &rhs,
                                  const PolynomialFactory& poly_factory, SymbolTable &symbol_table,
                                  Multithreading::MultiThreadPolicy policy) const {
        return do_raw_polynomial_multiply<false>(rhs, *this, poly_factory, symbol_table, policy);
    }



}