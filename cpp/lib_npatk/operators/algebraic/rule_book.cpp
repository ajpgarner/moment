/**
 * rule_book.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "rule_book.h"
#include "algebraic_context.h"

namespace NPATK {

    RuleBook::RuleBook(const AlgebraicContext& ac, std::vector<MonomialSubstitutionRule> rules)
         : context{ac}, monomialRules{std::move(rules)} {

    }

    void RuleBook::validate_rules() {

    }
}