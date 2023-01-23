/**
 * substituted_matrix.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include "symbolic_matrix.h"
#include "symbolic/substitution_list.h"

namespace Moment {

    class SubstitutedMatrix : public SymbolicMatrix {
    public:
        const SymbolicMatrix& source_matrix;

    private:
        std::unique_ptr<SubstitutionList> sub_list;
    public:
        SubstitutedMatrix(const Context& context, SymbolTable& symbols,
                          const SymbolicMatrix& source_matrix, std::unique_ptr<SubstitutionList> substitutions);

        const SubstitutionList& substitutions() const { return *sub_list; }

        std::string description() const override;
    };

}