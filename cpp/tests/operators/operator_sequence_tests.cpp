/*
 * (c) 2022-2022 Austrian Academy of Sciences.
 */
#include "gtest/gtest.h"

#include "operators/operator_sequence.h"
#include "operators/context.h"
#include "operators/locality/locality_context.h"

#include <list>
#include <vector>

namespace NPATK::Tests {
    TEST(OperatorSequence, Sequence_Empty) {
        Context empty{0};
        OperatorSequence seq{empty};
        EXPECT_TRUE(seq.empty());
        EXPECT_EQ(seq.size(), 0);
        EXPECT_EQ(seq.begin(), seq.end());
    }

    TEST(OperatorSequence, Sequence_OneOper) {
        Context empty{0};
        oper_name_t memA{3};
        OperatorSequence seq{{memA}, empty};
        ASSERT_FALSE(seq.empty());
        ASSERT_EQ(seq.size(), 1);
        auto iter = seq.begin();
        ASSERT_NE(iter, seq.end());
        EXPECT_EQ(*iter, memA);
        EXPECT_EQ(seq[0], *iter);

        ++iter;
        ASSERT_EQ(iter, seq.end());
    }

    TEST(OperatorSequence, Sequence_TwoSameParty) {
        Context empty{0};
        oper_name_t  memA{5};
        oper_name_t  memB{10};
        OperatorSequence seqAB{{memA, memB}, empty};
        ASSERT_FALSE(seqAB.empty());
        ASSERT_EQ(seqAB.size(), 2);
        auto iterAB = seqAB.begin();
        ASSERT_NE(iterAB, seqAB.end());
        EXPECT_EQ(*iterAB, memA);
        EXPECT_EQ(seqAB[0], *iterAB);
        ++iterAB;
        ASSERT_NE(iterAB, seqAB.end());
        EXPECT_EQ(*iterAB, memB);

        EXPECT_EQ(seqAB[1], *iterAB);
        ++iterAB;
        ASSERT_EQ(iterAB, seqAB.end());

        OperatorSequence seqBA{{memB, memA}, empty};
        ASSERT_FALSE(seqBA.empty());
        ASSERT_EQ(seqBA.size(), 2);
        auto iterBA = seqBA.begin();
        ASSERT_NE(iterBA, seqBA.end());
        EXPECT_EQ(*iterBA, memB);
        EXPECT_EQ(seqBA[0], *iterBA);
        ++iterBA;
        ASSERT_NE(iterBA, seqBA.end());
        EXPECT_EQ(*iterBA, memA);
        EXPECT_EQ(seqBA[1], *iterBA);
        ++iterBA;
        ASSERT_EQ(iterBA, seqBA.end());
    }

    TEST(OperatorSequence, Sequence_CompareEqual) {
        Context empty{0};
        oper_name_t  memA{5};
        oper_name_t  memB{10};
        OperatorSequence seqAB1{{memA, memB}, empty};
        OperatorSequence seqAB2{{memA, memB}, empty};
        OperatorSequence seqBA{{memB, memA}, empty};

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

    TEST(OperatorSequence, Sequence_ConjugateNonncommute) {
        Context empty{0};
        oper_name_t  memA{1};
        oper_name_t  memB{2};

        OperatorSequence seqAB{{memA, memB}, empty};
        OperatorSequence seqBA{{memB, memA}, empty};
        ASSERT_TRUE(seqAB != seqBA);

        auto conjugate = seqAB.conjugate();
        EXPECT_EQ(conjugate, seqBA);
    }

    TEST(OperatorSequence, Sequence_ConjugateIdentity) {
        Context empty{0};

        OperatorSequence id{{}, empty};
        OperatorSequence id2 = OperatorSequence::Identity(empty);
        ASSERT_EQ(id, id2);

        auto conjugate = id.conjugate();
        EXPECT_EQ(conjugate, id);
        EXPECT_EQ(conjugate, id2);

        auto conjugate2 = id2.conjugate();
        EXPECT_EQ(conjugate2, id);
        EXPECT_EQ(conjugate2, id2);
    }

    TEST(OperatorSequence, Sequence_ConjugateZero) {
        Context empty{0};

        OperatorSequence zero = OperatorSequence::Zero(empty);
        ASSERT_TRUE(zero.zero());
        auto conjugate = zero.conjugate();
        EXPECT_TRUE(conjugate.zero());
        EXPECT_EQ(conjugate, zero);
    }


    TEST(OperatorSequence, Sequence_Append_AB_listBBA) {
        Context empty{0};
        oper_name_t  memA{1};
        oper_name_t  memB{2};

        std::list<oper_name_t > appList{memB, memB, memA};

        OperatorSequence seq{{memA, memB}, empty};

        seq.append(appList.cbegin(), appList.cend());
        OperatorSequence seqABA{{memA, memB, memB, memB, memA}, empty};
        EXPECT_EQ(seq, seqABA);
    }

    TEST(OperatorSequence, Sequence_Append_ABC_initBBA) {
        Context empty{0};
        oper_name_t  memA{1};
        oper_name_t  memB{2};
        oper_name_t  memC{3};

        OperatorSequence seq{{memA, memB, memC}, empty};
        seq.append({memB, memB, memA});
        OperatorSequence seqABAC{{memA, memB, memC, memB, memB, memA}, empty};
        EXPECT_EQ(seq, seqABAC);
    }


    TEST(OperatorSequence, WithContext_MutexZero) {
        LocalityContext collection{Party::MakeList(1, 1, 4)};

        ASSERT_EQ(collection.Parties.size(), 1);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 3);
        ASSERT_TRUE(alice.mutually_exclusive(alice[1], alice[2]));
        ASSERT_TRUE(alice.mutually_exclusive(alice[2], alice[1]));

        OperatorSequence seq01({alice[0], alice[1]}, collection);
        ASSERT_EQ(seq01.size(), 0);
        EXPECT_TRUE(seq01.zero());

        OperatorSequence seq12({alice[1], alice[2]}, collection);
        ASSERT_EQ(seq12.size(), 0);
        EXPECT_TRUE(seq12.zero());

        OperatorSequence seq21({alice[2], alice[1]}, collection);
        ASSERT_EQ(seq21.size(), 0);
        EXPECT_TRUE(seq21.zero());

    }
}
