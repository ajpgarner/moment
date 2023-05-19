/**
 * substituted_matrix.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "monomial_matrix.h"
#include "polynomial_matrix.h"

#include "symbolic/substitution_list.h"
#include "symbolic/moment_substitution_rulebook.h"


namespace Moment {

    /**
     * Substituted monomial matrix
     */
    class SubstitutedMatrix : public MonomialMatrix {
    public:
        const MonomialMatrix& source_matrix;

    private:
        std::unique_ptr<SubstitutionList> sub_list;
    public:
        SubstitutedMatrix(const Context& context, SymbolTable& symbols,
                          const MonomialMatrix& source_matrix, std::unique_ptr<SubstitutionList> substitutions);

        const SubstitutionList& substitutions() const { return *sub_list; }

    };


    /**
     * Substituted monomial matrix.
     * Source matrix is always monomial.
     */
    class MonomialSubstitutedMatrix : public MonomialMatrix {
    public:
        const Matrix& source_matrix;
        const MomentSubstitutionRulebook& rules;

    public:
        MonomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                  const MonomialMatrix& source_matrix);


        /**
         * Forms a new monomial matrix by element-wise application of MSRB onto Matrix data.
         * @param msrb The rulebook of substitutions.
         * @param matrix The monomial matrix
         * @return
         */
        static std::unique_ptr<SquareMatrix<SymbolExpression>>
        reduce(const MomentSubstitutionRulebook& msrb, const SquareMatrix<SymbolExpression>& matrix);

    };


    /**
     * Substituted polynomial matrix.
     * Source matrix can be monomial or polynomial.
     */
    class PolynomialSubstitutedMatrix : public PolynomialMatrix {
    public:
        const Matrix& source_matrix;
        const MomentSubstitutionRulebook& rules;

    public:
        PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                    const MonomialMatrix& source_matrix);

        PolynomialSubstitutedMatrix(SymbolTable& symbols, const MomentSubstitutionRulebook& msrb,
                                    const PolynomialMatrix& source_matrix);

    public:
        static std::unique_ptr<SquareMatrix<SymbolCombo>>
        reduce(const MomentSubstitutionRulebook& msrb, const SquareMatrix<SymbolCombo>& matrix);

        static std::unique_ptr<SquareMatrix<SymbolCombo>>
        reduce(const MomentSubstitutionRulebook& msrb, const SquareMatrix<SymbolExpression>& matrix);
    };


}