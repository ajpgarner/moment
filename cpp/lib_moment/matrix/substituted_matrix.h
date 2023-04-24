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

    // TODO: Polynomial substituted matrix

}