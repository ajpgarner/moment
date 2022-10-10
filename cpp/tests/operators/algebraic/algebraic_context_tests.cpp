/**
 * algebraic_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/operator_sequence.h"
#include "operators/algebraic/algebraic_context.h"
#include "operators/operator_sequence_generator.h"

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

    TEST(AlgebraicContext, ContextTwoSubstitution_ABtoA_BAtoA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{1});
        rules.emplace_back(std::vector<oper_name_t>{2, 1}, std::vector<oper_name_t>{1});
        AlgebraicContext ac{3, std::move(rules)};

        ac.generate_aliases(4);

        OperatorSequence seq_AB{std::vector{Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 1);
        EXPECT_EQ(seq_AB[0].id, 1);

        OperatorSequence seq_BA{std::vector{Operator{2}, Operator{1}}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 1);
        EXPECT_EQ(seq_BA[0].id, 1);

        OperatorSequence seq_AAB{std::vector{Operator{1}, Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0].id, 1);
        EXPECT_EQ(seq_AAB[1].id, 1);

        OperatorSequence seq_BAB{std::vector{Operator{2}, Operator{1}, Operator{2}}, ac};
        EXPECT_FALSE(seq_BAB.empty());
        EXPECT_FALSE(seq_BAB.zero());
        ASSERT_EQ(seq_BAB.size(), 1);
        EXPECT_EQ(seq_BAB[0].id, 1);
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

    TEST(AlgebraicContext, ContextMakeGenerator_ABtoBA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{0, 1}, std::vector<oper_name_t>{1, 0});
        AlgebraicContext ac{2, std::move(rules)};
        ac.generate_aliases(4);

        OperatorSequenceGenerator osg_lvl1{ac, 1};
        ASSERT_EQ(osg_lvl1.size(), 3); // I, A, B
        auto osgIter1 = osg_lvl1.begin();
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({}, ac));

        ++osgIter1;
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({Operator{0}}, ac));

        ++osgIter1;
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({Operator{1}}, ac));

        ++osgIter1;
        ASSERT_EQ(osgIter1, osg_lvl1.end());
        
        OperatorSequenceGenerator osg_lvl2{ac, 2};
        ASSERT_EQ(osg_lvl2.size(), 6); // I, A, B, AA, AB, BB
        auto osgIter2 = osg_lvl2.begin();
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({Operator{0}}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({Operator{1}}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({Operator{0}, Operator{0}}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({Operator{0}, Operator{1}}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({Operator{1}, Operator{1}}, ac));

        ++osgIter2;
        ASSERT_EQ(osgIter2, osg_lvl2.end());

    }
}