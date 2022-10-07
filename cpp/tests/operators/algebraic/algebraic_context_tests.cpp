/**
 * algebraic_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/algebraic/algebraic_context.h"

namespace NPATK::Tests {

    TEST(AlgebraicContext, Empty) {
        AlgebraicContext ac{0};

        ac.generate_aliases(4);

    }

    TEST(AlgebraicContext, ContextOneSubstitution_ABtoA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{1});
        AlgebraicContext ac{3, std::move(rules)};

        ac.generate_aliases(3);


    }
}