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
        EXPECT_EQ(ac.size(), 0);

        EXPECT_NO_THROW(ac.generate_aliases(4));
    }

    TEST(AlgebraicContext, NoRules) {
        AlgebraicContext ac{2};
        EXPECT_EQ(ac.size(), 2);

        EXPECT_NO_THROW(ac.generate_aliases(4));
    }

    TEST(AlgebraicContext, OneSubstitution_ABtoA) {
        std::vector<MonomialSubstitutionRule> rules;


        rules.emplace_back(
                HashedSequence{{1, 2}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );
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
        ASSERT_EQ(seq_BA.size(), 1);
        EXPECT_EQ(seq_BA[0], 1);

        OperatorSequence seq_AAB{std::vector<oper_name_t>{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);
    }

    TEST(AlgebraicContext, TwoSubstitution_ABtoA_BAtoA) {
        std::vector<MonomialSubstitutionRule> rules;

        rules.emplace_back(
                HashedSequence{{1, 2}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );

        rules.emplace_back(
                HashedSequence{{2, 1}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );
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

    TEST(AlgebraicContext, TwoSubstitution_ABtoA_BAtoI) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(
                HashedSequence{{1, 2}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );

        rules.emplace_back(
                HashedSequence{{2, 1}, ShortlexHasher{3}},
                HashedSequence{{}, ShortlexHasher{3}}
        );

        AlgebraicContext ac{3, true, std::move(rules)};

        ac.generate_aliases(6);

        OperatorSequence seq_A{std::vector<oper_name_t>{1}, ac};
        EXPECT_TRUE(seq_A.empty());
        EXPECT_FALSE(seq_A.zero());
        EXPECT_EQ(seq_A.size(), 0);

        OperatorSequence seq_B{std::vector<oper_name_t>{2}, ac};
        EXPECT_TRUE(seq_B.empty());
        EXPECT_FALSE(seq_B.zero());
        EXPECT_EQ(seq_B.size(), 0);

        OperatorSequence seq_AB{std::vector<oper_name_t>{1, 2}, ac};
        EXPECT_TRUE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        EXPECT_EQ(seq_AB.size(), 0);

        OperatorSequence seq_BA{std::vector<oper_name_t>{2, 1}, ac};
        EXPECT_TRUE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        EXPECT_EQ(seq_BA.size(), 0);

        OperatorSequence seq_AAB{std::vector<oper_name_t>{1, 1, 2}, ac};
        EXPECT_TRUE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        EXPECT_EQ(seq_AAB.size(), 0);

        OperatorSequence seq_BAB{std::vector<oper_name_t>{2, 1, 2}, ac};
        EXPECT_TRUE(seq_BAB.empty());
        EXPECT_FALSE(seq_BAB.zero());
        EXPECT_EQ(seq_BAB.size(), 0);

    }

    TEST(AlgebraicContext, OneSubstitution_ABtoBA) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(
                HashedSequence{{2, 1}, ShortlexHasher{3}},
                HashedSequence{{1, 2}, ShortlexHasher{3}}
        );
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
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{0, 1}, ShortlexHasher{2}}
        );

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
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                HashedSequence{{0}, ShortlexHasher{2}}
        );
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{}, ShortlexHasher{2}}
        );

        AlgebraicContext ac{2, true, std::move(rules)};
        ac.generate_aliases(2);

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

    TEST(AlgebraicContext, MakeGenerator_ABtoA_BCtoB_CAtoA) {
        // AB=A, BC=B, CA=C -> A = B = C
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{3}},
                HashedSequence{{0}, ShortlexHasher{3}}
        );
        rules.emplace_back(
                HashedSequence{{1, 2}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );
        rules.emplace_back(
                HashedSequence{{2, 0}, ShortlexHasher{3}},
                HashedSequence{{2}, ShortlexHasher{3}}
        );

        AlgebraicContext ac{3, true, std::move(rules)};
        ac.generate_aliases(1);

        OperatorSequenceGenerator osg_lvl1{ac, 1};
        ASSERT_EQ(osg_lvl1.size(), 2); // I, a
        auto osgIter1 = osg_lvl1.begin();
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({}, ac));

        ++osgIter1;
        ASSERT_NE(osgIter1, osg_lvl1.end());
        EXPECT_EQ(*osgIter1, OperatorSequence({0}, ac));

        ++osgIter1;
        ASSERT_EQ(osgIter1, osg_lvl1.end());
    }

    TEST(AlgebraicContext, CreateMomentMatrix_ABtoI) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                HashedSequence{{}, ShortlexHasher{2}}
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(2, true, std::move(rules));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = ams.Context();

        auto& mm1 = ams.CreateMomentMatrix(1); // 1, A, B; A A^2 I; B I B^2 ...
        ASSERT_EQ(mm1.Level(), 1);
        EXPECT_TRUE(mm1.IsHermitian());
        const auto& seqMat = mm1.SequenceMatrix;
        ASSERT_EQ(seqMat.Dimension(), 3);
        EXPECT_EQ(seqMat[0][0], OperatorSequence::Identity(context));
        EXPECT_EQ(seqMat[0][1], OperatorSequence({0}, context));
        EXPECT_EQ(seqMat[0][2], OperatorSequence({1}, context));

        EXPECT_EQ(seqMat[1][0], OperatorSequence({0}, context));
        EXPECT_EQ(seqMat[1][1], OperatorSequence({0, 0}, context));
        EXPECT_EQ(seqMat[1][2], OperatorSequence::Identity(context));

        EXPECT_EQ(seqMat[2][0], OperatorSequence({1}, context));
        EXPECT_EQ(seqMat[2][1], OperatorSequence::Identity(context));
        EXPECT_EQ(seqMat[2][2], OperatorSequence({1, 1}, context));
    }

    TEST(AlgebraicContext, CreateMomentMatrix_ABtoA_BAtoI) {
        std::vector<MonomialSubstitutionRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                HashedSequence{{0}, ShortlexHasher{2}}
        );
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{}, ShortlexHasher{2}}
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(2, true, std::move(rules));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = dynamic_cast<const AlgebraicContext&>(ams.Context());

        auto& mm1 = ams.CreateMomentMatrix(1); // 1 (because A=1, B=1...!)
        ASSERT_EQ(mm1.Level(), 1);
        EXPECT_TRUE(mm1.IsHermitian());
        ASSERT_EQ(mm1.Dimension(), 1);
        EXPECT_EQ(mm1.SequenceMatrix[0][0], OperatorSequence::Identity(ams.Context()));

        auto& mm3 = ams.CreateMomentMatrix(3); // 1 (because A=1, B=1, still!)
        ASSERT_EQ(mm3.Level(), 3);
        EXPECT_TRUE(mm3.IsHermitian());
        ASSERT_EQ(mm3.Dimension(), 1) << context.resolved_rules();
        EXPECT_EQ(mm3.SequenceMatrix[0][0], OperatorSequence::Identity(ams.Context()));
    }
}
