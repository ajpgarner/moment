/**
 * rule_book.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "monomial_substitution_rule.h"

namespace NPATK {
    class AlgebraicContext;

    class RuleBook {
    private:
        const AlgebraicContext& context;
        std::vector<MonomialSubstitutionRule> monomialRules;

    public:
        RuleBook(const AlgebraicContext& context, std::vector<MonomialSubstitutionRule> rules);

    private:
        void validate_rules();

    };

}