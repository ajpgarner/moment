/**
 * polynomial_to_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include <Eigen/SparseCore>

#include "polynomial.h"
#include "utilities/dynamic_bitset.h"

namespace Moment {
    class SymbolTable;
    class Polynomial;

    using basis_vec_t = Eigen::SparseVector<double>;
    using complex_basis_vec_t = Eigen::SparseVector<std::complex<double>>;

    /**
     * Extract basis mask(s) from a polynomial
     */
    class PolynomialToMask {
    public:
        const SymbolTable& symbols;

        using MaskType = DynamicBitset<uint64_t, symbol_name_t>;

        const double zero_tolerance = 1.0;

    public:
        explicit PolynomialToMask(const SymbolTable& symbols, const double zero_tolerance)
            : symbols{symbols}, zero_tolerance{zero_tolerance} { }

        /** Get empty mask of correct size. */
        [[nodiscard]] std::pair<MaskType, MaskType> empty_mask() const;

        /** Set bits for real and imaginary basis elements of supplied polynomial. */
        void set_bits(MaskType& real_mask, MaskType& imaginary_mask,
                      const Polynomial& poly) const;

        /** Set bits for real and imaginary basis elements of supplied monomial. */
        void set_bits(MaskType& real_mask, MaskType& imaginary_mask,
                      const Monomial& mono) const;

        /** Get masks for real and imaginary basis elements of supplied polynomial */
        [[nodiscard]] inline std::pair<MaskType, MaskType>
        operator()(const Polynomial& poly) const {
            auto output = this->empty_mask();
            this->set_bits(output.first, output.second, poly);
            return output;
        }

        /** Convert bitmasks to sets. */
        [[nodiscard]] static std::pair<std::set<symbol_name_t>, std::set<symbol_name_t>>
        masks_to_sets(const MaskType& real_mask, const MaskType& imaginary_mask);
    };

    /**
     * Convert a Polynomial into a vector of basis co-efficients.
     */
    class PolynomialToBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit PolynomialToBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<basis_vec_t, basis_vec_t> operator()(const Polynomial& poly) const;
    };

    /**
     * Convert a Polynomial into a vector of complex basis co-efficients.
     */
    class PolynomialToComplexBasisVec {
    public:
        const SymbolTable& symbols;

    public:
        explicit PolynomialToComplexBasisVec(const SymbolTable& symbols) : symbols{symbols} { }

        std::pair<complex_basis_vec_t, complex_basis_vec_t> operator()(const Polynomial& poly) const;
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