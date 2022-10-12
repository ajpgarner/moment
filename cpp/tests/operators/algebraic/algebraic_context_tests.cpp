/**
 * algebraic_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/operator_sequence.h"
#include "operators/operator_sequence_generator.h"
#include "operators/matrix/moment_matrix.h"
#include "operators/algebraic/algebraic_context.h"
#include "operators/algebraic/algebraic_matrix_system.h"

namespace NPATK::Tests {

    TEST(AlgebraicContext, Empty) {
        AlgebraicContext ac{0};

        ac.generate_aliases(4);

    }

    TEST(AlgebraicContext, OneSubstitution_ABtoA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{1});
        AlgebraicContext ac{3, true, std::move(rules)};

        ac.generate_aliases(3);

        OperatorSequence seq_AB{std::vector<oper_name_t>{1, 2}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 1);
        EXPECT_EQ(seq_AB[0], 1);

        OperatorSequence seq_BA{std::vector<oper_name_t>{2, 1}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 2);
        EXPECT_EQ(seq_BA[0], 2);
        EXPECT_EQ(seq_BA[1], 1);

        OperatorSequence seq_AAB{std::vector<oper_name_t>{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);
    }

    TEST(AlgebraicContext, TwoSubstitution_ABtoA_BAtoA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{1});
        rules.emplace_back(std::vector<oper_name_t>{2, 1}, std::vector<oper_name_t>{1});
        AlgebraicContext ac{3, true, std::move(rules)};

        ac.generate_aliases(4);

        OperatorSequence seq_AB{std::vector<oper_name_t>{1, 2}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 1);
        EXPECT_EQ(seq_AB[0], 1);

        OperatorSequence seq_BA{std::vector<oper_name_t>{2, 1}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 1);
        EXPECT_EQ(seq_BA[0], 1);

        OperatorSequence seq_AAB{std::vector<oper_name_t>{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);

        OperatorSequence seq_BAB{std::vector<oper_name_t>{2, 1, 2}, ac};
        EXPECT_FALSE(seq_BAB.empty());
        EXPECT_FALSE(seq_BAB.zero());
        ASSERT_EQ(seq_BAB.size(), 1);
        EXPECT_EQ(seq_BAB[0], 1);
    }

    TEST(AlgebraicContext, OneSubstitution_ABtoBA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{1, 2}, std::vector<oper_name_t>{2, 1});
        AlgebraicContext ac{3, true, std::move(rules)};

        ac.generate_aliases(3);

        OperatorSequence seq_AB{std::vector<oper_name_t>{1, 2}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 2);
        EXPECT_EQ(seq_AB[0], 1);
        EXPECT_EQ(seq_AB[1], 2);

        OperatorSequence seq_BA{std::vector<oper_name_t>{2, 1}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 2);
        EXPECT_EQ(seq_BA[0], 1);
        EXPECT_EQ(seq_BA[1], 2);

        OperatorSequence seq_AAB{std::vector<oper_name_t>{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 3);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);
        EXPECT_EQ(seq_AAB[2], 2);

        OperatorSequence seq_ABA{std::vector<oper_name_t>{1, 2, 1}, ac};
        EXPECT_FALSE(seq_ABA.empty());
        EXPECT_FALSE(seq_ABA.zero());
        ASSERT_EQ(seq_ABA.size(), 3);
        EXPECT_EQ(seq_ABA[0], 1);
        EXPECT_EQ(seq_ABA[1], 1);
        EXPECT_EQ(seq_ABA[2], 2);

        OperatorSequence seq_BAA{std::vector<oper_name_t>{2, 1, 1}, ac};
        EXPECT_FALSE(seq_BAA.empty());
        EXPECT_FALSE(seq_BAA.zero());
        ASSERT_EQ(seq_BAA.size(), 3);
        EXPECT_EQ(seq_BAA[0], 1);
        EXPECT_EQ(seq_BAA[1], 1);
        EXPECT_EQ(seq_BAA[2], 2);
    }

    TEST(AlgebraicContext, MakeGenerator_ABtoBA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{0, 1}, std::vector<oper_name_t>{1, 0});
        AlgebraicContext ac{2, true, std::move(rules)};
        ac.generate_aliases(4);

        OperatorSequenceGenerator osg_lvl1{ac, 1};
        ASSERT_EQ(osg_lvl1.size(), 3); // I, A, B
        auto osgIter1 = osg_lvl1.begin();
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({}, ac));

        ++osgIter1;
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({0}, ac));

        ++osgIter1;
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({1}, ac));

        ++osgIter1;
        ASSERT_EQ(osgIter1, osg_lvl1.end());
        
        OperatorSequenceGenerator osg_lvl2{ac, 2};
        ASSERT_EQ(osg_lvl2.size(), 6); // I, A, B, AA, AB, BB
        auto osgIter2 = osg_lvl2.begin();
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({0}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({1}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({0, 0}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({0, 1}, ac));

        ++osgIter2;
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({1, 1}, ac));

        ++osgIter2;
        ASSERT_EQ(osgIter2, osg_lvl2.end());

    }

    TEST(AlgebraicContext, MakeGenerator_ABtoA_BAtoI) {
        // AB=A, BA=1; but AB=A implies BA=A and hence A=1, and hence B=1.
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{0, 1}, std::vector<oper_name_t>{0});
        rules.emplace_back(std::vector<oper_name_t>{1, 0}, std::vector<oper_name_t>{});
        AlgebraicContext ac{2, true, std::move(rules)};
        ac.generate_aliases(4);

        OperatorSequenceGenerator osg_lvl1{ac, 1};
        ASSERT_EQ(osg_lvl1.size(), 1); // I
        auto osgIter1 = osg_lvl1.begin();
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({}, ac));

        ++osgIter1;
        ASSERT_EQ(osgIter1, osg_lvl1.end());

        OperatorSequenceGenerator osg_lvl2{ac, 2};
        ASSERT_EQ(osg_lvl2.size(), 1); // I
        auto osgIter2 = osg_lvl2.begin();
        ASSERT_NE(osgIter2, osg_lvl2.end());
        EXPECT_EQ(*osgIter2, OperatorSequence({}, ac));

        ++osgIter2;
        ASSERT_EQ(osgIter2, osg_lvl2.end());

    }

    TEST(AlgebraicContext, CreateMomentMatrix_ABtoI) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(std::vector<oper_name_t>{0, 1}, std::vector<oper_name_t>{});
        auto ac_ptr = std::make_unique<AlgebraicContext>(2, true, std::move(rules));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        auto& mm1 = ams.CreateMomentMatrix(1); // 1, A, B; A A^2 I; B I B^2 ...
        ASSERT_EQ(mm1.Level(), 1);
        EXPECT_TRUE(mm1.IsHermitian());
    }

    TEST(AlgebraicContext, CreateMomentMatrix_ABtoA_BAtoI) {
//        std::vector<MonomialSubstitutionRule> rules;
//        rules.emplace_back(std::vector<oper_name_t>{0, 1}, std::vector<oper_name_t>{0});
//        rules.emplace_back(std::vector<oper_name_t>{1, 0}, std::vector<oper_name_t>{});
//        auto ac_ptr = std::make_unique<AlgebraicContext>(2, true, std::move(rules));
//        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
//        auto& mm1 = ams.CreateMomentMatrix(1); // 1 (because A=1, B=1...!)
//        ASSERT_EQ(mm1.Level(), 1);
//        EXPECT_TRUE(mm1.IsHermitian());
    }
}
