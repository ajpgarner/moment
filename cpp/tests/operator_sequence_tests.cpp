/**
 * operator_sequence_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/operator_sequence.h"

namespace NPATK::Tests {
    TEST(OperatorSequence, Sequence_Empty) {
        OperatorSequence seq{};
        EXPECT_TRUE(seq.empty());
        EXPECT_EQ(seq.size(), 0);
        EXPECT_EQ(seq.begin(), seq.end());
    }

    TEST(OperatorSequence, Sequence_OneOper) {
        Operator memA{3, Party{17}};
        OperatorSequence seq{memA};
        ASSERT_FALSE(seq.empty());
        ASSERT_EQ(seq.size(), 1);
        auto iter = seq.begin();
        ASSERT_NE(iter, seq.end());
        EXPECT_EQ(*iter, memA);
        ++iter;
        ASSERT_EQ(iter, seq.end());
    }

    TEST(OperatorSequence, Sequence_TwoSameParty) {
        Operator memA{5, Party{1}};
        Operator memB{10, Party{1}};
        OperatorSequence seqAB{memA, memB};
        ASSERT_FALSE(seqAB.empty());
        ASSERT_EQ(seqAB.size(), 2);
        auto iterAB = seqAB.begin();
        ASSERT_NE(iterAB, seqAB.end());
        EXPECT_EQ(*iterAB, memA);
        ++iterAB;
        ASSERT_NE(iterAB, seqAB.end());
        EXPECT_EQ(*iterAB, memB);
        ++iterAB;
        ASSERT_EQ(iterAB, seqAB.end());

        OperatorSequence seqBA{memB, memA};
        ASSERT_FALSE(seqBA.empty());
        ASSERT_EQ(seqBA.size(), 2);
        auto iterBA = seqBA.begin();
        ASSERT_NE(iterBA, seqBA.end());
        EXPECT_EQ(*iterBA, memB);
        ++iterBA;
        ASSERT_NE(iterBA, seqBA.end());
        EXPECT_EQ(*iterBA, memA);
        ++iterBA;
        ASSERT_EQ(iterBA, seqBA.end());
    }


    TEST(OperatorSequence, Sequence_TwoDiffParty) {
        Operator memA{5, Party{1}};
        Operator memB{10, Party{2}};
        OperatorSequence seqAB{memA, memB};
        ASSERT_FALSE(seqAB.empty());
        ASSERT_EQ(seqAB.size(), 2);
        auto iterAB = seqAB.begin();
        ASSERT_NE(iterAB, seqAB.end());
        EXPECT_EQ(*iterAB, memA);
        ++iterAB;
        ASSERT_NE(iterAB, seqAB.end());
        EXPECT_EQ(*iterAB, memB);
        ++iterAB;
        ASSERT_EQ(iterAB, seqAB.end());

        OperatorSequence seqBA{memB, memA};
        ASSERT_FALSE(seqBA.empty());
        ASSERT_EQ(seqBA.size(), 2);
        auto iterBA = seqBA.begin();
        ASSERT_NE(iterBA, seqBA.end());
        EXPECT_EQ(*iterBA, memA);
        ++iterBA;
        ASSERT_NE(iterBA, seqBA.end());
        EXPECT_EQ(*iterBA, memB);
        ++iterBA;
        ASSERT_EQ(iterBA, seqBA.end());
    }

    TEST(OperatorSequence, Sequence_CompareEqual) {
        Operator memA{5, Party{1}};
        Operator memB{10, Party{1}};
        OperatorSequence seqAB1{memA, memB};
        OperatorSequence seqAB2{memA, memB};
        OperatorSequence seqBA{memB, memA};

        EXPECT_TRUE(seqAB1 == seqAB1);
        EXPECT_TRUE(seqAB1 == seqAB2);
        EXPECT_FALSE(seqAB1 == seqBA);

        EXPECT_TRUE(seqAB2 == seqAB1);
        EXPECT_TRUE(seqAB2 == seqAB2);
        EXPECT_FALSE(seqAB2 == seqBA);

        EXPECT_FALSE(seqBA == seqAB1);
        EXPECT_FALSE(seqBA == seqAB2);
        EXPECT_TRUE(seqBA == seqBA);
    }


    TEST(OperatorSequence, Sequence_IdemAAA) {
        Operator memA{5, Party{1}, Operator::Flags::Idempotent};
        OperatorSequence seqA{memA};
        OperatorSequence seqAA{memA, memA};
        OperatorSequence seqAAA{memA, memA, memA};

        ASSERT_EQ(seqA.size(), 1);
        EXPECT_EQ(seqAA.size(), 1);
        EXPECT_EQ(seqAAA.size(), 1);

        EXPECT_EQ(seqA, seqAA);
        EXPECT_EQ(seqAA, seqAAA);
    }

    TEST(OperatorSequence, Sequence_IdemAAABB) {
        Operator memA{5, Party{1}, Operator::Flags::Idempotent};
        Operator memB{10, Party{1}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqAAABB{memA, memA, memA, memB, memB};

        ASSERT_EQ(seqAB.size(), 2);
        EXPECT_EQ(seqAAABB.size(), 2);

        EXPECT_EQ(seqAB, seqAAABB);
    }

    TEST(OperatorSequence, Sequence_IdemAAABB2) {
        Operator memA{5, Party{1}, Operator::Flags::Idempotent};
        Operator memB{5, Party{2}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqAAABB{memA, memA, memA, memB, memB};

        ASSERT_EQ(seqAB.size(), 2);
        EXPECT_EQ(seqAAABB.size(), 2);

        EXPECT_EQ(seqAB, seqAAABB);
    }

    TEST(OperatorSequence, Sequence_ConjugateCommute) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{2}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        auto conjugate = seqAB.conjugate();
        EXPECT_EQ(conjugate, seqAB);
    }

    TEST(OperatorSequence, Sequence_ConjugateNonncommute) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqBA{memB, memA};
        ASSERT_TRUE(seqAB != seqBA);

        auto conjugate = seqAB.conjugate();
        EXPECT_EQ(conjugate, seqBA);
    }


    TEST(OperatorSequence, Sequence_Append_AB_listBBA) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};

        std::list<Operator> appList{memB, memB, memA};

        OperatorSequence seq{memA, memB};

        seq.append(appList.cbegin(), appList.cend());
        OperatorSequence seqABA{memA, memB, memA};
        EXPECT_EQ(seq, seqABA);
    }

    TEST(OperatorSequence, Sequence_Append_AB_vecBBA) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};

        std::vector<Operator> appVec{memB, memB, memA};

        OperatorSequence seq{memA, memB};

        seq.append(appVec.cbegin(), appVec.cend());
        OperatorSequence seqABA{memA, memB, memA};
        EXPECT_EQ(seq, seqABA);
    }

    TEST(OperatorSequence, Sequence_Append_ABC_initBBA) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};
        Operator memC{3, Party{2}, Operator::Flags::Idempotent};

        OperatorSequence seq{memA, memB, memC};
        seq.append({memB, memB, memA});
        OperatorSequence seqABAC{memA, memB, memA, memC};
        EXPECT_EQ(seq, seqABAC);
    }

    TEST(OperatorSequence, Sequence_Concat_AB_AB) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqABAB{memA, memB, memA, memB};

        auto concat = seqAB * seqAB;
        EXPECT_EQ(concat, seqABAB);
    }


    TEST(OperatorSequence, Sequence_Concat_ABconj_AB) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqBAB{memB, memA, memB};

        auto concat = seqAB.conjugate() * seqAB;
        EXPECT_EQ(concat, seqBAB);
    }

    TEST(OperatorSequence, Sequence_Concat_AB_ABconj) {
        Operator memA{1, Party{1}, Operator::Flags::Idempotent};
        Operator memB{2, Party{1}, Operator::Flags::Idempotent};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqABA{memA, memB, memA};

        auto concat = seqAB * seqAB.conjugate();
        EXPECT_EQ(concat, seqABA);
    }
}