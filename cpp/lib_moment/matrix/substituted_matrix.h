/**
 * substituted_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "monomial_matrix.h"
#include "polynomial_matrix.h"

#include "symbolic/moment_substitution_rulebook.h"


namespace Moment {

    /**
     * Substituted monomial matrix
     */
    class SubstitutedMatrix {
    public:
        const Matrix& source_matrix;
        const MomentSubstitutionRulebook& rules;

    public:
        explicit SubstitutedMatrix(const Matrix& source, const MomentSubstitutionRulebook& rules) noexcept
            : source_matrix{source}, rules{rules} { }
    };


    /**
     * Substituted monomial matrix.
     * Source matrix is always monomial.
     */
    class MonomialSubstitutedMatrix : public MonomialMatrix, public SubstitutedMatrix {
    public:
        MonomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                  const MonomialMatrix& source_matrix);


        /**
         * Forms a new monomial matrix by element-wise application of MSRB onto Matrix data.
         * @param msrb The rulebook of substitutions.
         * @param matrix The raw monomial source matrix.
         * @return Newly created raw monomial matrix.
         */
        static std::unique_ptr<SquareMatrix<Monomial>>
        reduce(const MomentSubstitutionRulebook& msrb, const SquareMatrix<Monomial>& matrix);

    };


    /**
     * Substituted polynomial matrix.
     * Source matrix can be monomial or polynomial.
     */
    class PolynomialSubstitutedMatrix : public PolynomialMatrix, public SubstitutedMatrix {
    public:
        PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                    const MonomialMatrix& source_matrix);

        PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                    const PolynomialMatrix& source_matrix);

    public:
        static std::unique_ptr<SquareMatrix<SymbolCombo>>
        reduce(const MomentSubstitutionRulebook& msrb, const SquareMatrix<SymbolCombo>& matrix);

        static std::unique_ptr<SquareMatrix<SymbolCombo>>
        reduce(const MomentSubstitutionRulebook& msrb, const SquareMatrix<Monomial>& matrix);
    };


}