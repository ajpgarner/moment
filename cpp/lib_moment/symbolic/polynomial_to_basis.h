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
        double zero_tolerance;

    public:
        explicit PolynomialToBasisVec(const SymbolTable& symbols, double zero_tolerance)
            : symbols{symbols}, zero_tolerance{zero_tolerance} { }

        [[nodiscard]] std::pair<basis_vec_t, basis_vec_t> operator()(const Polynomial& poly) const;

        /**
         * Add polynomial as a row in triplets (for later synthesizing into sparse matrix).
         * Real and imaginary basis elements are added to different vectors.
         * @param poly The polynomial to write
         * @param row_index The row index associated (column given by basis).
         * @param real_triplets Real element triplets to write into.
         * @param im_triplets Imaginary element triplets to write into.
         */
        void add_triplet_row(const Polynomial& poly, Eigen::Index row_index,
                             std::vector<Eigen::Triplet<double>>& real_triplets,
                             std::vector<Eigen::Triplet<double>>& im_triplets) const;

        /**
         * Add polynomial as a row in triplets (for later synthesizing into sparse matrix).
         * Real and imaginary basis elements are added to the same vector, but with an offset for imaginary terms.
         * @param poly The polynomial to write
         * @param row_index The row index associated (column given by basis).
         * @param combined_triplets Triplets object to write into.
         * @param im_offset Column offset for imaginary terms (typically == number of real elements in symbol table).
         */
        void add_triplet_row(const Polynomial& poly, Eigen::Index row_index,
                             std::vector<Eigen::Triplet<double>>& combined_triplets,
                             Eigen::Index im_offset) const;
    };

    /**
     * Convert a Polynomial into a vector of complex basis co-efficients.
     */
    class PolynomialToComplexBasisVec {
    public:
        const SymbolTable& symbols;
        double zero_tolerance;

    public:
        explicit PolynomialToComplexBasisVec(const SymbolTable& symbols, double zero_tolerance)
            : symbols{symbols}, zero_tolerance{zero_tolerance} { }

        [[nodiscard]] std::pair<complex_basis_vec_t, complex_basis_vec_t> operator()(const Polynomial& poly) const;

        /**
        * Add polynomial as a row in triplets (for later synthesizing into sparse matrix).
        * Real and imaginary basis elements are added to different vectors.
        * @param poly The polynomial to write
        * @param row_index The row index associated (column given by basis).
        * @param real_triplets Real element triplets to write into.
        * @param im_triplets Imaginary element triplets to write into.
        */
        void add_triplet_row(const Polynomial& poly, Eigen::Index row_index,
                             std::vector<Eigen::Triplet<std::complex<double>>>& real_triplets,
                             std::vector<Eigen::Triplet<std::complex<double>>>& im_triplets) const;

        /**
         * Add polynomial as a row in triplets (for later synthesizing into sparse matrix).
         * Real and imaginary basis elements are added to the same vector, but with an offset for imaginary terms.
         * @param poly The polynomial to write
         * @param row_index The row index associated (column given by basis).
         * @param combined_triplets Triplets object to write into.
         * @param im_offset Column offset for imaginary terms (typically == number of real elements in symbol table).
         */
        void add_triplet_row(const Polynomial& poly, Eigen::Index row_index,
                             std::vector<Eigen::Triplet<std::complex<double>>>& combined_triplets,
                             Eigen::Index im_offset) const;
    };

    /**
     * Convert a vector of basis co-efficients into a Polynomial.
     */
    class BasisVecToPolynomial {
    public:
        const PolynomialFactory& factory;

    public:
        explicit BasisVecToPolynomial(const PolynomialFactory& factory) : factory{factory} { }

        [[nodiscard]] Polynomial operator()(const basis_vec_t& real, const basis_vec_t& img) const;
    };

    /**
     * Convert a vector of complex basis co-efficients into a Polynomial.
     */
    class ComplexBasisVecToPolynomial {
    public:
        const PolynomialFactory& factory;

    public:
        explicit ComplexBasisVecToPolynomial(const PolynomialFactory& factory) : factory{factory} { }

        [[nodiscard]] Polynomial operator()(const complex_basis_vec_t& real, const complex_basis_vec_t& img) const;
    };



}