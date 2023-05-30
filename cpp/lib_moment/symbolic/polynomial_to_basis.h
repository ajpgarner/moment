/**
 * polynomial_to_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/SparseCore>

#include "polynomial.h"

namespace Moment {
    class SymbolTable;
    class Polynomial;

    using basis_vec_t = Eigen::SparseVector<double>;
    using complex_basis_vec_t = Eigen::SparseVector<std::complex<double>>;

    /**
     * Convert a Polynomial into a vector of basis co-efficients.
     */
    class PolynomialToBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit PolynomialToBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<basis_vec_t, basis_vec_t> operator()(const Polynomial& combo) const;
    };

    /**
     * Convert a Polynomial into a vector of complex basis co-efficients.
     */
    class PolynomialToComplexBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit PolynomialToComplexBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<complex_basis_vec_t, complex_basis_vec_t> operator()(const Polynomial& combo) const;
    };

    /**
     * Convert a vector of basis co-efficients into a Polynomial.
     */
    class BasisVecToPolynomial {
    public:
        const SymbolTable& symbols;

    public:
        explicit BasisVecToPolynomial(const SymbolTable& symbols) : symbols{symbols} { }

        Polynomial operator()(const basis_vec_t& real, const basis_vec_t& img) const;
    };

    /**
     * Convert a vector of complex basis co-efficients into a Polynomial.
     */
    class ComplexBasisVecToPolynomial {
    public:
        const SymbolTable& symbols;

    public:
        explicit ComplexBasisVecToPolynomial(const SymbolTable& symbols) : symbols{symbols} { }

        Polynomial operator()(const complex_basis_vec_t& real, const complex_basis_vec_t& img) const;
    };



}