/**
 * polynomial_matrix_arithmetic.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "polynomial_matrix.h"
#include "composite_matrix.h"
#include "monomial_matrix.h"

namespace Moment {


    std::unique_ptr<PolynomialMatrix> PolynomialMatrix::add(const Monomial& rhs, const PolynomialFactory& poly_factory,
                                                         Multithreading::MultiThreadPolicy policy) const {
        // Special case: add zero
        if ((rhs.id == 0) || (approximately_zero(rhs.factor, poly_factory.zero_tolerance))) {
            auto clone_ptr = this->clone(policy);

            // Promote up...!
            assert(clone_ptr->is_polynomial());
            return std::unique_ptr<PolynomialMatrix>{dynamic_cast<PolynomialMatrix*>(clone_ptr.release())};

        }

        // Otherwise, construct a polynomial matrix from this matrix

        // General case: add a polynomial
        std::vector<Polynomial> output_polynomials;
        output_polynomials.reserve(this->dimension * this->dimension);
        for (auto matrix_elem : *this->sym_exp_matrix) {
            output_polynomials.emplace_back(poly_factory.sum(matrix_elem, rhs));
        }
        auto output_poly_sm = std::make_unique<SquareMatrix<Polynomial>>(this->dimension, std::move(output_polynomials));

        // Construct new polynomial matrix
        return std::make_unique<PolynomialMatrix>(this->context, this->symbol_table, poly_factory.zero_tolerance,
                                                  std::move(output_poly_sm));
    }


    std::unique_ptr<PolynomialMatrix> PolynomialMatrix::add(const Polynomial& rhs, const PolynomialFactory& poly_factory,
                                                          Multithreading::MultiThreadPolicy policy) const {

        // Special case: add zero
        if (rhs.empty()) {
            auto clone_ptr = this->clone(policy);

            // Promote up...!
            assert(clone_ptr->is_polynomial());
            return std::unique_ptr<PolynomialMatrix>{dynamic_cast<PolynomialMatrix*>(clone_ptr.release())};
        }

        // Special case: add monomial
        if (rhs.is_monomial()) {
            assert (!rhs.empty());
            return this->add(rhs.back(), poly_factory, policy);
        }

        // General case: add a polynomial
        std::vector<Polynomial> output_polynomials;
        output_polynomials.reserve(this->dimension * this->dimension);
        for (auto matrix_elem : *this->sym_exp_matrix) {
            output_polynomials.emplace_back(poly_factory.sum(matrix_elem, rhs));
        }
        auto output_poly_sm = std::make_unique<SquareMatrix<Polynomial>>(this->dimension, std::move(output_polynomials));

        // Construct new polynomial matrix
        return std::make_unique<PolynomialMatrix>(this->context, this->symbol_table, poly_factory.zero_tolerance,
                                                  std::move(output_poly_sm));
    }



}