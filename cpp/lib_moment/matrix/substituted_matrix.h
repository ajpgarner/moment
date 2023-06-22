/**
 * substituted_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "monomial_matrix.h"
#include "polynomial_matrix.h"

#include "symbolic/rules/moment_rulebook.h"


namespace Moment {

    /**
     * Substituted monomial matrix
     */
    class SubstitutedMatrix {
    public:
        const Matrix& source_matrix;
        const MomentRulebook& rules;

    public:
        explicit SubstitutedMatrix(const Matrix& source, const MomentRulebook& rules) noexcept
            : source_matrix{source}, rules{rules} { }

    protected:
        [[nodiscard]] std::string make_name();
    };


    /**
     * Substituted monomial matrix.
     * Source matrix is always monomial.
     */
    class MonomialSubstitutedMatrix : public MonomialMatrix, public SubstitutedMatrix {
    public:
        MonomialSubstitutedMatrix(SymbolTable& symbols, const MomentRulebook& msrb,
                                  const MonomialMatrix& source_matrix);


        /**
         * Forms a new monomial matrix by element-wise application of MSRB onto Matrix data.
         * @param msrb The rulebook of substitutions.
         * @param matrix The raw monomial source matrix.
         * @return Newly created raw monomial matrix.
         */
        static std::unique_ptr<SquareMatrix<Monomial>>
        reduce(const MomentRulebook& msrb, const SquareMatrix<Monomial>& matrix);



    };


    /**
     * Substituted polynomial matrix.
     * Source matrix can be monomial or polynomial.
     */
    class PolynomialSubstitutedMatrix : public PolynomialMatrix, public SubstitutedMatrix {
    public:
        PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentRulebook& msrb,
                                    const MonomialMatrix& source_matrix);

        PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentRulebook& msrb,
                                    const PolynomialMatrix& source_matrix);

    public:
        static std::unique_ptr<SquareMatrix<Polynomial>>
        reduce(const MomentRulebook& msrb, const SquareMatrix<Polynomial>& matrix);

        static std::unique_ptr<SquareMatrix<Polynomial>>
        reduce(const MomentRulebook& msrb, const SquareMatrix<Monomial>& matrix);
    };


}