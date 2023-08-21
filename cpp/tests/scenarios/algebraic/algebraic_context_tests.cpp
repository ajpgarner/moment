/**
 * algebraic_context_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "dictionary/operator_sequence_generator.h"
#include "matrix/operator_matrix/moment_matrix.h"
#include "dictionary/operator_sequence.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Scenarios_Algebraic_AlgebraicContext, Empty) {
        AlgebraicContext ac{0};
        EXPECT_EQ(ac.size(), 0);

    }

    TEST(Scenarios_Algebraic_AlgebraicContext, NoRules) {
        AlgebraicContext ac{2};
        EXPECT_EQ(ac.size(), 2);

    }

    TEST(Scenarios_Algebraic_AlgebraicContext, OneSubstitution_ABtoA) {
        std::vector<OperatorRule> rules;

        AlgebraicPrecontext apc{3, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        rules.emplace_back(
                HashedSequence{{1, 2}, apc.hasher},
                HashedSequence{{1}, apc.hasher}
        );
        AlgebraicContext ac{apc, false, true, rules};
        ASSERT_TRUE(ac.attempt_completion(20));

        OperatorSequence seq_AB{sequence_storage_t{1, 2}, ac};
        EXPECT_FALSE(seq_AB.empty()) << seq_AB;
        EXPECT_FALSE(seq_AB.zero()) << seq_AB;
        ASSERT_EQ(seq_AB.size(), 1) << seq_AB;
        EXPECT_EQ(seq_AB[0], 1) << seq_AB;

        OperatorSequence seq_BA{sequence_storage_t{2, 1}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 1);
        EXPECT_EQ(seq_BA[0], 1);

        OperatorSequence seq_AAB{sequence_storage_t{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);
    }

    TEST(Scenarios_Algebraic_AlgebraicContext, TwoSubstitution_ABtoA_BAtoA) {
        std::vector<OperatorRule> rules;

        rules.emplace_back(
                HashedSequence{{1, 2}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );

        rules.emplace_back(
                HashedSequence{{2, 1}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );
        AlgebraicContext ac{AlgebraicPrecontext{3}, false, true, rules};
        ASSERT_TRUE(ac.attempt_completion(20));

        OperatorSequence seq_AB{sequence_storage_t{1, 2}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 1);
        EXPECT_EQ(seq_AB[0], 1);

        OperatorSequence seq_BA{sequence_storage_t{2, 1}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 1);
        EXPECT_EQ(seq_BA[0], 1);

        OperatorSequence seq_AAB{sequence_storage_t{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 2);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);

        OperatorSequence seq_BAB{sequence_storage_t{2, 1, 2}, ac};
        EXPECT_FALSE(seq_BAB.empty());
        EXPECT_FALSE(seq_BAB.zero());
        ASSERT_EQ(seq_BAB.size(), 1);
        EXPECT_EQ(seq_BAB[0], 1);
    }

    TEST(Scenarios_Algebraic_AlgebraicContext, TwoSubstitution_ABtoA_BAtoI) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{1, 2}, ShortlexHasher{3}},
                HashedSequence{{1}, ShortlexHasher{3}}
        );

        rules.emplace_back(
                HashedSequence{{2, 1}, ShortlexHasher{3}},
                HashedSequence{{}, ShortlexHasher{3}}
        );

        AlgebraicContext ac{AlgebraicPrecontext{3}, false, true, rules};
        ASSERT_TRUE(ac.attempt_completion(20));

        OperatorSequence seq_A{sequence_storage_t{1}, ac};
        EXPECT_TRUE(seq_A.empty());
        EXPECT_FALSE(seq_A.zero());
        EXPECT_EQ(seq_A.size(), 0);

        OperatorSequence seq_B{sequence_storage_t{2}, ac};
        EXPECT_TRUE(seq_B.empty());
        EXPECT_FALSE(seq_B.zero());
        EXPECT_EQ(seq_B.size(), 0);

        OperatorSequence seq_AB{sequence_storage_t{1, 2}, ac};
        EXPECT_TRUE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        EXPECT_EQ(seq_AB.size(), 0);

        OperatorSequence seq_BA{sequence_storage_t{2, 1}, ac};
        EXPECT_TRUE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        EXPECT_EQ(seq_BA.size(), 0);

        OperatorSequence seq_AAB{sequence_storage_t{1, 1, 2}, ac};
        EXPECT_TRUE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        EXPECT_EQ(seq_AAB.size(), 0);

        OperatorSequence seq_BAB{sequence_storage_t{2, 1, 2}, ac};
        EXPECT_TRUE(seq_BAB.empty());
        EXPECT_FALSE(seq_BAB.zero());
        EXPECT_EQ(seq_BAB.size(), 0);

    }

    TEST(Scenarios_Algebraic_AlgebraicContext, OneSubstitution_ABtoBA) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{2, 1}, ShortlexHasher{3}},
                HashedSequence{{1, 2}, ShortlexHasher{3}}
        );
        AlgebraicContext ac{AlgebraicPrecontext{3}, false, true, rules};

        OperatorSequence seq_AB{sequence_storage_t{1, 2}, ac};
        EXPECT_FALSE(seq_AB.empty());
        EXPECT_FALSE(seq_AB.zero());
        ASSERT_EQ(seq_AB.size(), 2);
        EXPECT_EQ(seq_AB[0], 1);
        EXPECT_EQ(seq_AB[1], 2);

        OperatorSequence seq_BA{sequence_storage_t{2, 1}, ac};
        EXPECT_FALSE(seq_BA.empty());
        EXPECT_FALSE(seq_BA.zero());
        ASSERT_EQ(seq_BA.size(), 2);
        EXPECT_EQ(seq_BA[0], 1);
        EXPECT_EQ(seq_BA[1], 2);

        OperatorSequence seq_AAB{sequence_storage_t{1, 1, 2}, ac};
        EXPECT_FALSE(seq_AAB.empty());
        EXPECT_FALSE(seq_AAB.zero());
        ASSERT_EQ(seq_AAB.size(), 3);
        EXPECT_EQ(seq_AAB[0], 1);
        EXPECT_EQ(seq_AAB[1], 1);
        EXPECT_EQ(seq_AAB[2], 2);

        OperatorSequence seq_ABA{sequence_storage_t{1, 2, 1}, ac};
        EXPECT_FALSE(seq_ABA.empty());
        EXPECT_FALSE(seq_ABA.zero());
        ASSERT_EQ(seq_ABA.size(), 3);
        EXPECT_EQ(seq_ABA[0], 1);
        EXPECT_EQ(seq_ABA[1], 1);
        EXPECT_EQ(seq_ABA[2], 2);

        OperatorSequence seq_BAA{sequence_storage_t{2, 1, 1}, ac};
        EXPECT_FALSE(seq_BAA.empty());
        EXPECT_FALSE(seq_BAA.zero());
        ASSERT_EQ(seq_BAA.size(), 3);
        EXPECT_EQ(seq_BAA[0], 1);
        EXPECT_EQ(seq_BAA[1], 1);
        EXPECT_EQ(seq_BAA[2], 2);
    }

    TEST(Scenarios_Algebraic_AlgebraicContext, OneSubstitution_ABtoMinusBA) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                true
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(AlgebraicPrecontext{2}, false, true, std::move(rules));
        const auto& context = *ac_ptr;

        OperatorSequence seq_AB{sequence_storage_t{0, 1}, context};
        ASSERT_EQ(seq_AB.size(), 2);
        EXPECT_EQ(seq_AB[0], 0);
        EXPECT_EQ(seq_AB[1], 1);
        EXPECT_FALSE(seq_AB.negated());

        OperatorSequence seq_minusAB{sequence_storage_t{0, 1}, context, true};
        ASSERT_EQ(seq_minusAB.size(), 2);
        EXPECT_EQ(seq_minusAB[0], 0);
        EXPECT_EQ(seq_minusAB[1], 1);
        EXPECT_TRUE(seq_minusAB.negated());

        OperatorSequence seq_ABstar = seq_AB.conjugate();
        ASSERT_EQ(seq_ABstar.size(), 2);
        EXPECT_EQ(seq_ABstar[0], 0);
        EXPECT_EQ(seq_ABstar[1], 1);
        EXPECT_TRUE(seq_ABstar.negated());

        OperatorSequence seq_BA{sequence_storage_t{1, 0}, context};
        ASSERT_EQ(seq_BA.size(), 2);
        EXPECT_EQ(seq_BA[0], 0);
        EXPECT_EQ(seq_BA[1], 1);
        EXPECT_TRUE(seq_BA.negated());

        OperatorSequence seq_BAstar = seq_BA.conjugate(); // BA = -AB, BA.conj -> (-AB).conj = (-BA) = AB
        ASSERT_EQ(seq_BAstar.size(), 2);
        EXPECT_EQ(seq_BAstar[0], 0);
        EXPECT_EQ(seq_BAstar[1], 1);
        EXPECT_FALSE(seq_BAstar.negated());

        OperatorSequence seq_minusBA{sequence_storage_t{1, 0}, context, true};
        ASSERT_EQ(seq_minusBA.size(), 2);
        EXPECT_EQ(seq_minusBA[0], 0);
        EXPECT_EQ(seq_minusBA[1], 1);
        EXPECT_FALSE(seq_minusBA.negated());

        OperatorSequence seq_ABA{sequence_storage_t{0, 1, 0}, context};
        ASSERT_EQ(seq_ABA.size(), 3);
        EXPECT_EQ(seq_ABA[0], 0);
        EXPECT_EQ(seq_ABA[1], 0);
        EXPECT_EQ(seq_ABA[2], 1);
        EXPECT_TRUE(seq_ABA.negated());

        OperatorSequence seq_ABAA{sequence_storage_t{0, 1, 0, 0}, context};
        ASSERT_EQ(seq_ABAA.size(), 4);
        EXPECT_EQ(seq_ABAA[0], 0);
        EXPECT_EQ(seq_ABAA[1], 0);
        EXPECT_EQ(seq_ABAA[2], 0);
        EXPECT_EQ(seq_ABAA[3], 1);
        EXPECT_FALSE(seq_ABAA.negated());

    }

    TEST(Scenarios_Algebraic_AlgebraicContext, MakeGenerator_ABtoBA) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{0, 1}, ShortlexHasher{2}}
        );

        AlgebraicContext ac{AlgebraicPrecontext{2}, false, true, rules};

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

    TEST(Scenarios_Algebraic_AlgebraicContext, MakeGenerator_ABtoA_BAtoI) {
        // AB=A, BA=1; but AB=A implies BA=A and hence A=1, and hence B=1.
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                HashedSequence{{0}, ShortlexHasher{2}}
        );
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{}, ShortlexHasher{2}}
        );

        AlgebraicContext ac{AlgebraicPrecontext{2}, false, true, rules};
        ASSERT_TRUE(ac.attempt_completion(20));

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

    TEST(Scenarios_Algebraic_AlgebraicContext, MakeGenerator_ABtoA_BCtoB_CAtoA) {
        // AB=A, BC=B, CA=C -> A = B = C
        std::vector<OperatorRule> rules;
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

        AlgebraicContext ac{AlgebraicPrecontext{3}, false, true, rules};
        ASSERT_TRUE(ac.attempt_completion(20));

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

    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_ABtoI) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                HashedSequence{{}, ShortlexHasher{2}}
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(AlgebraicPrecontext{2}, false, true, std::move(rules));
        ASSERT_TRUE(ac_ptr->attempt_completion(20));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = ams.Context();


        auto& mm1 = ams.MomentMatrix(1); // 1, A, B; A A^2 I; B I B^2 ...
        EXPECT_TRUE(mm1.Hermitian());
        const auto& seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;
        ASSERT_EQ(seqMat.Level(), 1);
        ASSERT_EQ(seqMat.Dimension(), 3);
        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(context));
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, context));
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, context));

        EXPECT_EQ(seqMat(1, 0), OperatorSequence({0}, context));
        EXPECT_EQ(seqMat(1, 1), OperatorSequence({0, 0}, context));
        EXPECT_EQ(seqMat(1, 2), OperatorSequence::Identity(context));

        EXPECT_EQ(seqMat(2, 0), OperatorSequence({1}, context));
        EXPECT_EQ(seqMat(2, 1), OperatorSequence::Identity(context));
        EXPECT_EQ(seqMat(2, 2), OperatorSequence({1, 1}, context));
    }

    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_ABtoA_BAtoI) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                HashedSequence{{0}, ShortlexHasher{2}}
        );
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{}, ShortlexHasher{2}}
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(AlgebraicPrecontext{2}, false, true, std::move(rules));
        ASSERT_TRUE(ac_ptr->attempt_completion(20));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = dynamic_cast<const AlgebraicContext&>(ams.Context());

        auto& mm1 = ams.MomentMatrix(1); // 1 (because A=1, B=1...!)
        const auto* seqMat1Ptr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMat1Ptr, nullptr);
        const auto& seqMat1 = *seqMat1Ptr;
        ASSERT_EQ(seqMat1.Level(), 1);
        EXPECT_TRUE(mm1.Hermitian());
        ASSERT_EQ(mm1.Dimension(), 1);
        EXPECT_EQ(seqMat1(0, 0), OperatorSequence::Identity(ams.Context()));

        auto& mm3 = ams.MomentMatrix(3); // 1 (because A=1, B=1, still!)
        const auto* seqMat3Ptr = MomentMatrix::as_monomial_moment_matrix_ptr(mm3);
        ASSERT_NE(seqMat3Ptr, nullptr);
        const auto& seqMat3 = *seqMat3Ptr;
        ASSERT_EQ(seqMat3.Level(), 3);
        EXPECT_TRUE(mm3.Hermitian());
        ASSERT_EQ(mm3.Dimension(), 1) << context.resolved_rules();
        EXPECT_EQ(seqMat3(0, 0), OperatorSequence::Identity(ams.Context()));

    }

    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_AAtoA) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{0, 0}, ShortlexHasher{2}},
                HashedSequence{{0}, ShortlexHasher{2}}
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(AlgebraicPrecontext{2}, false, true, std::move(rules));
        ASSERT_TRUE(ac_ptr->attempt_completion(20));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = dynamic_cast<const AlgebraicContext&>(ams.Context());

        auto& mm2 = ams.MomentMatrix(2); // 1 a b ab ba bb
        const auto* seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm2);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;

        ASSERT_EQ(seqMat.Level(), 2);
        EXPECT_TRUE(mm2.Hermitian());
        ASSERT_EQ(mm2.Dimension(), 6);
        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(ams.Context()));
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, ams.Context()));
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, ams.Context()));
        EXPECT_EQ(seqMat(0, 3), OperatorSequence({0, 1}, ams.Context()));
        EXPECT_EQ(seqMat(0, 4), OperatorSequence({1, 0}, ams.Context()));
        EXPECT_EQ(seqMat(0, 5), OperatorSequence({1, 1}, ams.Context()));
    }

    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_ABtoMinusBA) {
        std::vector<OperatorRule> rules;
        rules.emplace_back(
                HashedSequence{{1, 0}, ShortlexHasher{2}},
                HashedSequence{{0, 1}, ShortlexHasher{2}},
                true
        );
        auto ac_ptr = std::make_unique<AlgebraicContext>(AlgebraicPrecontext{2}, false, true, std::move(rules));
        ASSERT_TRUE(ac_ptr->attempt_completion(20));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = dynamic_cast<const AlgebraicContext&>(ams.Context());

        auto& mm1 = ams.MomentMatrix(1); // 1 a b
        const auto* seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;
        ASSERT_EQ(seqMat.Level(), 1);
        EXPECT_TRUE(mm1.Hermitian());
        ASSERT_EQ(mm1.Dimension(), 3);
        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(ams.Context()));
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, ams.Context()));
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, ams.Context()));
        EXPECT_EQ(seqMat(1, 0), OperatorSequence({0}, ams.Context()));
        EXPECT_EQ(seqMat(1, 1), OperatorSequence({0, 0}, ams.Context()));
        EXPECT_EQ(seqMat(1, 2), OperatorSequence({0, 1}, ams.Context()));
        EXPECT_EQ(seqMat(2, 0), OperatorSequence({1}, ams.Context()));
        EXPECT_EQ(seqMat(2, 1), OperatorSequence({0, 1}, ams.Context(), true));
        EXPECT_EQ(seqMat(2, 2), OperatorSequence({1, 1}, ams.Context()));

        // Check symbols
        const auto& symTable = ams.Symbols();
        const auto x0x1Ptr = symTable.where(OperatorSequence({0, 1}, ams.Context()));
        ASSERT_NE(x0x1Ptr, nullptr);
        const auto& x0x1 = *x0x1Ptr;
        EXPECT_TRUE(x0x1.is_antihermitian()) << symTable;
        auto [rePart, imPart] = x0x1.basis_key();
        EXPECT_FALSE(x0x1.is_hermitian()) << symTable;
        EXPECT_EQ(rePart, -1) << symTable;
        EXPECT_NE(imPart, -1) << symTable;
    }


    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_Commutative) {
        ShortlexHasher hasher{2};
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{0, 1}, hasher},
                         HashedSequence{{0}, hasher}); // AB-> A

        auto ac_ptr = std::make_unique<AlgebraicContext>(AlgebraicPrecontext{2}, true, true, std::move(msr));
        ASSERT_TRUE(ac_ptr->attempt_completion(20));
        AlgebraicMatrixSystem ams{std::move(ac_ptr)};
        const auto& context = ams.AlgebraicContext();

        auto& mm1 = ams.MomentMatrix(1); // 1 a b
        const auto* seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;

        ASSERT_EQ(seqMat.Level(), 1);
        EXPECT_TRUE(mm1.Hermitian());
        ASSERT_EQ(mm1.Dimension(), 3);
        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(ams.Context()));
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, ams.Context()));
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, ams.Context()));
        EXPECT_EQ(seqMat(1, 0), OperatorSequence({0}, ams.Context()));
        EXPECT_EQ(seqMat(1, 1), OperatorSequence({0, 0}, ams.Context()));
        EXPECT_EQ(seqMat(1, 2), OperatorSequence({0}, ams.Context()));
        EXPECT_EQ(seqMat(2, 0), OperatorSequence({1}, ams.Context()));
        EXPECT_EQ(seqMat(2, 1), OperatorSequence({0}, ams.Context(), true));
        EXPECT_EQ(seqMat(2, 2), OperatorSequence({1, 1}, ams.Context()));

    }


    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_NonHermitian) {
        AlgebraicPrecontext apc{1, AlgebraicPrecontext::ConjugateMode::Bunched};
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(apc, false, false)};
        const auto& context = ams.AlgebraicContext();
        ASSERT_EQ(context.size(), 2); // a, a*

        auto& mm1 = ams.MomentMatrix(1); // 1 a a*
        const auto* seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;

        ASSERT_EQ(seqMat.Level(), 1);
        EXPECT_TRUE(mm1.Hermitian());
        ASSERT_EQ(mm1.Dimension(), 3);

        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(context)); // 1
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, context));      // a
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, context));      // a*
        EXPECT_EQ(seqMat(1, 0), OperatorSequence({1}, context));      // a*
        EXPECT_EQ(seqMat(1, 1), OperatorSequence({1, 0}, context));   // a* a
        EXPECT_EQ(seqMat(1, 2), OperatorSequence({1, 1}, context));   // a* a*
        EXPECT_EQ(seqMat(2, 0), OperatorSequence({0}, context));   // a
        EXPECT_EQ(seqMat(2, 1), OperatorSequence({0, 0}, context));   // a a
        EXPECT_EQ(seqMat(2, 2), OperatorSequence({0, 1}, context));   // a a*

        const auto& symbols = ams.Symbols();
        ASSERT_EQ(symbols.size(), 6); // 0, 1, a<->a*, aa<->a*a*, a*a, aa*
    }

    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_NonHermitian_Normal) {
        AlgebraicPrecontext apc{1, AlgebraicPrecontext::ConjugateMode::Bunched};
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(apc, false, true)};
        const auto& context = ams.AlgebraicContext();
        ASSERT_EQ(context.size(), 2); // a, a*

        auto& mm1 = ams.MomentMatrix(1); // 1 a a*
        const auto* seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;
        ASSERT_EQ(seqMat.Level(), 1);

        EXPECT_TRUE(mm1.Hermitian());
        ASSERT_EQ(mm1.Dimension(), 3);

        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(context)); // 1
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, context));      // a
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, context));      // a*
        EXPECT_EQ(seqMat(1, 0), OperatorSequence({1}, context));      // a*
        EXPECT_EQ(seqMat(1, 1), OperatorSequence({0, 1}, context));   // a a *
        EXPECT_EQ(seqMat(1, 2), OperatorSequence({1, 1}, context));   // a* a*
        EXPECT_EQ(seqMat(2, 0), OperatorSequence({0}, context));   // a
        EXPECT_EQ(seqMat(2, 1), OperatorSequence({0, 0}, context));   // a a
        EXPECT_EQ(seqMat(2, 2), OperatorSequence({0, 1}, context));   // a a*

        const auto& symbols = ams.Symbols();
        ASSERT_EQ(symbols.size(), 5); // 0, 1, a<->a*, aa<->a*a*, aa*
    }


    TEST(Scenarios_Algebraic_AlgebraicContext, CreateMomentMatrix_AntiCommutator) {
        AlgebraicPrecontext apc{2, AlgebraicPrecontext::ConjugateMode::SelfAdjoint};
        std::vector<OperatorRule> msr;
        msr.emplace_back(HashedSequence{{1, 0}, apc.hasher},
                         HashedSequence{{0, 1}, apc.hasher}, true);

        auto contextPtr = std::make_unique<AlgebraicContext>(apc, false, true, std::move(msr));
        EXPECT_TRUE(contextPtr->attempt_completion(0));


        AlgebraicMatrixSystem ams{std::move(contextPtr)};
        auto& context = ams.AlgebraicContext();

        ASSERT_TRUE(context.is_complete());

        auto& mm1 = ams.MomentMatrix(1); // 1 x y
        const auto* seqMatPtr = MomentMatrix::as_monomial_moment_matrix_ptr(mm1);
        ASSERT_NE(seqMatPtr, nullptr);
        const auto& seqMat = *seqMatPtr;
        ASSERT_EQ(seqMat.Level(), 1);

        EXPECT_TRUE(mm1.Hermitian());
        ASSERT_EQ(mm1.Dimension(), 3);

        EXPECT_EQ(seqMat(0, 0), OperatorSequence::Identity(context)); // 1
        EXPECT_EQ(seqMat(0, 1), OperatorSequence({0}, context));      // x
        EXPECT_EQ(seqMat(0, 2), OperatorSequence({1}, context));      // y
        EXPECT_EQ(seqMat(1, 0), OperatorSequence({0}, context));      // x
        EXPECT_EQ(seqMat(1, 1), OperatorSequence({0, 0}, context));   // x^2
        EXPECT_EQ(seqMat(1, 2), OperatorSequence({0, 1}, context));   // x y
        EXPECT_EQ(seqMat(2, 0), OperatorSequence({1}, context));   // y
        EXPECT_EQ(seqMat(2, 1), OperatorSequence({0, 1}, context, true));   // -x y
        EXPECT_EQ(seqMat(2, 2), OperatorSequence({1, 1}, context));   // y y

        const auto& symbols = ams.Symbols();


    }
}
