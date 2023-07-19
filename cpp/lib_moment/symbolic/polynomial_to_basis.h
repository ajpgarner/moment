/**
 * polynomial_to_basis.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial.h"

#include <Eigen/SparseCore>

#include <span>
#include <vector>

namespace Moment {
    class SymbolTable;
    class Polynomial;

    using basis_vec_t = Eigen::SparseVector<double>;
    using complex_basis_vec_t = Eigen::SparseVector<std::complex<double>>;
    using complex_matrix_t = Eigen::SparseMatrix<std::complex<double>>;

    /**
     * Specification of a real number, in terms of real coefficients to multiply 'a' and 'b' by.
     */
    struct RealBasisVector {
        /** The (real) co-efficients to multiply the 'a' vector, associated with Hermitian terms, with. */
        basis_vec_t real;
        /** The (real) co-efficients to multiply the 'b' vector, associated with anti-Hermitian terms, with. */
        basis_vec_t imaginary;
    };

    /**
     * Specification of two real numbers (representing real and imaginary parts of a whole), each in terms of real
     * coefficients to multiply 'a' and 'b' by.
     */
    struct RealAndImaginaryBasisVector {
        /** The specification of the real part of the expression, in terms of real coefficients of a and b */
        RealBasisVector real_part;
        /** The specification of the imaginary part of the expression, in terms of real coefficients of a and b */
        RealBasisVector imaginary_part;
    };

    /**
     * Specification of a complex number each in terms of complex coefficients to multiply 'a' and 'b' by.
     */
    struct ComplexBasisVector {
        /** The complex coefficients, with which to multiply the 'a' vector, associated with Hermitian terms. */
        complex_basis_vec_t real;
        /** The complex coefficients, with which to multiply the 'b' vector, associated with anti-Hermitian terms. */
        complex_basis_vec_t imaginary;

        ComplexBasisVector() = default;
        explicit ComplexBasisVector(const RealAndImaginaryBasisVector& expr) {
            this->real = (std::complex<double>{0.0, 1.0} * expr.real_part.real)
                            + std::complex<double>{0.0, 1.0} * expr.imaginary_part.real;
            this->imaginary = (std::complex<double>{1.0, 0.0} * expr.real_part.imaginary)
                            + std::complex<double>{0.0, 1.0} * expr.imaginary_part.imaginary;
        }
    };

    struct ComplexMonolith {
        /** The complex coefficients, with which to post-multiply the 'a' row-vector, to yield the value per polynomial. */
        complex_matrix_t real;

        /** The complex coefficients, with which to post-multiply the 'b' row-vector, to yield the value per polynomial. */
        complex_matrix_t imaginary;

    public:
        /** Empty constructor */
        ComplexMonolith() = default;

        /** Construct empty monoliths, of specified dimensions. */
        ComplexMonolith(const size_t columns, const size_t real_rows, const size_t imaginary_rows)
            :    real(real_rows, columns), imaginary(imaginary_rows, columns) { }

        /** Construct empty monoliths, of specified dimensions, with sorted data. */
        ComplexMonolith(const size_t columns, const size_t real_rows, const size_t imaginary_rows,
                        std::span<Eigen::Triplet<std::complex<double>>> real_data,
                        std::span<Eigen::Triplet<std::complex<double>>> imaginary_data)
            : ComplexMonolith(columns, real_rows, imaginary_rows) {
            this->real.setFromSortedTriplets(real_data.begin(), real_data.end());
            this->imaginary.setFromSortedTriplets(imaginary_data.begin(), imaginary_data.end());
        }
    };

    /**
     * Convert a Polynomial into a pair of basis co-efficient vectors.
     *
     */
    class PolynomialToBasisVec {
    public:
        const SymbolTable& symbols;
        double zero_tolerance;

    public:
        explicit PolynomialToBasisVec(const SymbolTable& symbols, double zero_tolerance)
            : symbols{symbols}, zero_tolerance{zero_tolerance} { }

        /** Returns entire specification of potentially complex polynomial */
        [[nodiscard]] RealAndImaginaryBasisVector operator()(const Polynomial& poly) const;

        /** Returns only real part of polynomial */
        [[nodiscard]] RealBasisVector Real(const Polynomial& poly) const;

        /** Returns only imaginary part of polynomial */
        [[nodiscard]] RealBasisVector Imaginary(const Polynomial& poly) const;

        /**
         * Add polynomial as a row in triplets (for later synthesizing into sparse matrix).
         * Real and imaginary basis elements are added to different vectors.
         * @param poly The polynomial to write
         * @param row_index The row index associated (column given by basis).
         * @param real_triplets Real element triplets to write into.
         * @param im_triplets Imaginary element triplets to write into.
         */
        void add_triplet_row(const Polynomial& poly, Eigen::Index real_row_index, Eigen::Index imaginary_row_index,
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
        void add_triplet_row(const Polynomial& poly, Eigen::Index real_row_index, Eigen::Index imaginary_row_index,
                             std::vector<Eigen::Triplet<double>>& combined_triplets) const;
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

        [[nodiscard]] ComplexBasisVector operator()(const Polynomial& poly) const;

        /**
         * Monolith, where each column maps onto an element of some last-index-major object.
         * Number of rows = number of basis elements, number of cols = number of polynomials.
         * @param multi_poly
         * @return
         */
        [[nodiscard]] ComplexMonolith operator()(std::span<const Polynomial> multi_poly) const;
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