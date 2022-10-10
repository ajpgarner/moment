/**
 * algebraic_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/operator_sequence.h"
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

        OperatorSequence seq_AB{std::vector{Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 1);
        EXPECT_EQ(seq_AB[0].id, 1);

        OperatorSequence seq_BA{std::vector{Operator{2}, Operator{1}}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 2);
        EXPECT_EQ(seq_BA[0].id, 2);
        EXPECT_EQ(seq_BA[1].id, 1);

        OperatorSequence seq_AAB{std::vector{Operator{1}, Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0].id, 1);
        EXPECT_EQ(seq_AAB[1].id, 1);
    }

    TEST(AlgebraicContext, ContextOneSubstitution_ABtoBA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{2, 1});
        AlgebraicContext ac{3, std::move(rules)};

        ac.generate_aliases(3);

        OperatorSequence seq_AB{std::vector{Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 2);
        EXPECT_EQ(seq_AB[0].id, 1);
        EXPECT_EQ(seq_AB[1].id, 2);

        OperatorSequence seq_BA{std::vector{Operator{2}, Operator{1}}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 2);
        EXPECT_EQ(seq_BA[0].id, 1);
        EXPECT_EQ(seq_BA[1].id, 2);

        OperatorSequence seq_AAB{std::vector{Operator{1}, Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 3);
        EXPECT_EQ(seq_AAB[0].id, 1);
        EXPECT_EQ(seq_AAB[1].id, 1);
        EXPECT_EQ(seq_AAB[2].id, 2);

        OperatorSequence seq_ABA{std::vector{Operator{1}, Operator{2}, Operator{1}}, ac};
        EXPECT_FALSE(seq_ABA.empty());
        EXPECT_FALSE(seq_ABA.zero());
        ASSERT_EQ(seq_ABA.size(), 3);
        EXPECT_EQ(seq_ABA[0].id, 1);
        EXPECT_EQ(seq_ABA[1].id, 1);
        EXPECT_EQ(seq_ABA[2].id, 2);

        OperatorSequence seq_BAA{std::vector{Operator{2}, Operator{1}, Operator{1}}, ac};
        EXPECT_FALSE(seq_BAA.empty());
        EXPECT_FALSE(seq_BAA.zero());
        ASSERT_EQ(seq_BAA.size(), 3);
        EXPECT_EQ(seq_BAA[0].id, 1);
        EXPECT_EQ(seq_BAA[1].id, 1);
        EXPECT_EQ(seq_BAA[2].id, 2);
    }
}