/**
 * operator_sequence_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operator_sequence.h"

namespace NPATK::Tests {

    TEST(OperatorSequence, Party_Construct) {
        Party party{3};
        EXPECT_EQ(party.id, 3);
    }

    TEST(OperatorSequence, Party_CompareEqual) {
        Party pA{1}, pB{1}, pC{3};
        EXPECT_TRUE(pA == pA);
        EXPECT_TRUE(pA == pB);
        EXPECT_FALSE(pA == pC);

        EXPECT_TRUE(pB == pA);
        EXPECT_TRUE(pB == pB);
        EXPECT_FALSE(pB == pC);

        EXPECT_FALSE(pC == pA);
        EXPECT_FALSE(pC == pB);
        EXPECT_TRUE(pC == pC);
    }

    TEST(OperatorSequence, Party_CompareNotEqual) {
        Party pA{1}, pB{1}, pC{3};
        EXPECT_FALSE(pA != pA);
        EXPECT_FALSE(pA != pB);
        EXPECT_TRUE(pA != pC);

        EXPECT_FALSE(pB != pA);
        EXPECT_FALSE(pB != pB);
        EXPECT_TRUE(pB != pC);

        EXPECT_TRUE(pC != pA);
        EXPECT_TRUE(pC != pB);
        EXPECT_FALSE(pC != pC);
    }

    TEST(OperatorSequence, Party_CompareLess) {
        Party pA{1}, pB{1}, pC{3};
        EXPECT_FALSE(pA < pA);
        EXPECT_FALSE(pA < pB);
        EXPECT_TRUE(pA < pC);

        EXPECT_FALSE(pB < pA);
        EXPECT_FALSE(pB < pB);
        EXPECT_TRUE(pB < pC);

        EXPECT_FALSE(pC < pA);
        EXPECT_FALSE(pC < pB);
        EXPECT_FALSE(pC < pC);
    }

    TEST(OperatorSequence, Operator_Construct) {
        Operator test_op{13, Party{4}};
        EXPECT_EQ(test_op.id, 13);
        EXPECT_EQ(test_op.party, Party{4});
    }


    TEST(OperatorSequence, Operator_CompareEqual) {
        Operator opA1{13, Party{4}};
        Operator opA2{13, Party{4}};
        Operator opB{13, Party{5}};
        Operator opC{14, Party{4}};

        EXPECT_TRUE(opA1 == opA2);
        EXPECT_TRUE(opA2 == opA1);
        EXPECT_FALSE(opA1 == opB);
        EXPECT_FALSE(opB == opA1);
        EXPECT_FALSE(opA1 == opC);
        EXPECT_FALSE(opC == opA1);

    }

    TEST(OperatorSequence, Operator_CompareNotEqual) {
        Operator opA1{13, Party{4}};
        Operator opA2{13, Party{4}};
        Operator opB{13, Party{5}};
        Operator opC{14, Party{4}};

        EXPECT_FALSE(opA1 != opA2);
        EXPECT_FALSE(opA2 != opA1);
        EXPECT_TRUE(opA1 != opB);
        EXPECT_TRUE(opB != opA1);
        EXPECT_TRUE(opA1 != opC);
        EXPECT_TRUE(opC != opA1);
    }


    TEST(OperatorSequence, Operator_ComparePartyLess) {
        Operator opA1{13, Party{4}};
        Operator opA2{13, Party{4}};
        Operator opB{13, Party{5}};
        Operator opC{12, Party{5}};

        Operator::PartyComparator comp{};

        EXPECT_FALSE(comp(opA1, opA1));
        EXPECT_FALSE(comp(opA1, opA2));
        EXPECT_TRUE(comp(opA1, opB));
        EXPECT_TRUE(comp(opA1, opC));

        EXPECT_FALSE(comp(opA2, opA1));
        EXPECT_FALSE(comp(opA2, opA2));
        EXPECT_TRUE(comp(opA2, opB));
        EXPECT_TRUE(comp(opA2, opC));

        EXPECT_FALSE(comp(opB, opA1));
        EXPECT_FALSE(comp(opB, opA2));
        EXPECT_FALSE(comp(opB, opB));
        EXPECT_FALSE(comp(opB, opC));

        EXPECT_FALSE(comp(opC, opA1));
        EXPECT_FALSE(comp(opC, opA2));
        EXPECT_FALSE(comp(opC, opB));
        EXPECT_FALSE(comp(opC, opC));
    }



    TEST(OperatorSequence, Operator_CompareRedundant) {
        Operator opA_idem{1, Party{1}, true};
        Operator opA_non{1, Party{1}, false};
        Operator opB{1, Party{1}, true};
        Operator opC{2, Party{1}, true};
        Operator opD{1, Party{2}, true};

        Operator::IsRedundant comp{};

        EXPECT_TRUE(comp(opA_idem, opA_idem));
        EXPECT_FALSE(comp(opA_non, opA_non));
        EXPECT_TRUE(comp(opB, opB));
        EXPECT_TRUE(comp(opC, opC));
        EXPECT_TRUE(comp(opD, opD));

        // Note: comp(opA_idem, opA_non) is undefined.

        EXPECT_TRUE(comp(opA_idem, opB));
        EXPECT_FALSE(comp(opA_idem, opC));
        EXPECT_FALSE(comp(opA_idem, opD));
        // EXPECT_FALSE(comp(opA_non, opB)); <- not defined, as symbols differ only by idempotency.
        EXPECT_FALSE(comp(opA_non, opC));
        EXPECT_FALSE(comp(opA_non, opD));

        EXPECT_TRUE(comp(opB, opA_idem));
        EXPECT_FALSE(comp(opC, opA_idem));
        EXPECT_FALSE(comp(opD, opA_idem));
        // EXPECT_FALSE(comp(opB, opA_non)); <- not defined, as symbols differ only by idempotency.
        EXPECT_FALSE(comp(opC, opA_non));
        EXPECT_FALSE(comp(opD, opA_non));
    }


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
        Operator memA{5, Party{1}, true};
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
        Operator memA{5, Party{1}, true};
        Operator memB{10, Party{1}, true};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqAAABB{memA, memA, memA, memB, memB};

        ASSERT_EQ(seqAB.size(), 2);
        EXPECT_EQ(seqAAABB.size(), 2);

        EXPECT_EQ(seqAB, seqAAABB);
    }

    TEST(OperatorSequence, Sequence_IdemAAABB2) {
        Operator memA{5, Party{1}, true};
        Operator memB{5, Party{2}, true};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqAAABB{memA, memA, memA, memB, memB};

        ASSERT_EQ(seqAB.size(), 2);
        EXPECT_EQ(seqAAABB.size(), 2);

        EXPECT_EQ(seqAB, seqAAABB);
    }

    TEST(OperatorSequence, Sequence_ConjugateCommute) {
        Operator memA{1, Party{1}, true};
        Operator memB{2, Party{2}, true};

        OperatorSequence seqAB{memA, memB};
        auto conjugate = seqAB.conjugate();
        EXPECT_EQ(conjugate, seqAB);
    }

    TEST(OperatorSequence, Sequence_ConjugateNonncommute) {
        Operator memA{1, Party{1}, true};
        Operator memB{2, Party{1}, true};

        OperatorSequence seqAB{memA, memB};
        OperatorSequence seqBA{memB, memA};
        ASSERT_TRUE(seqAB != seqBA);

        auto conjugate = seqAB.conjugate();
        EXPECT_EQ(conjugate, seqBA);
    }



}