/*
 * (c) 2022-2022 Austrian Academy of Sciences.
 */
#include "gtest/gtest.h"

#include "scenarios/operator_sequence.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/locality/locality_context.h"

#include <list>
#include <vector>

namespace Moment::Tests {
    TEST(Operators_OperatorSequence, Sequence_Empty) {
        Context empty{0};
        OperatorSequence seq{empty};
        EXPECT_TRUE(seq.empty());
        EXPECT_EQ(seq.size(), 0);
        EXPECT_EQ(seq.begin(), seq.end());
    }

    TEST(Operators_OperatorSequence, Sequence_OneOper) {
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

    TEST(Operators_OperatorSequence, Sequence_TwoSameParty) {
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

    TEST(Operators_OperatorSequence, Sequence_CompareEqual) {
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

    TEST(Operators_OperatorSequence, Sequence_ConjugateNonncommute) {
        Context empty{0};
        oper_name_t  memA{1};
        oper_name_t  memB{2};

        OperatorSequence seqAB{{memA, memB}, empty};
        OperatorSequence seqBA{{memB, memA}, empty};
        ASSERT_TRUE(seqAB != seqBA);

        auto conjugate = seqAB.conjugate();
        EXPECT_EQ(conjugate, seqBA);
    }

    TEST(Operators_OperatorSequence, Sequence_ConjugateIdentity) {
        Context empty{1};

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

    TEST(Operators_OperatorSequence, Sequence_ConjugateZero) {
        Context empty{0};

        OperatorSequence zero = OperatorSequence::Zero(empty);
        ASSERT_TRUE(zero.zero());
        auto conjugate = zero.conjugate();
        EXPECT_TRUE(conjugate.zero());
        EXPECT_EQ(conjugate, zero);
    }


    TEST(Operators_OperatorSequence, Sequence_Append_AB_listBBA) {
        Context empty{0};
        oper_name_t  memA{1};
        oper_name_t  memB{2};

        std::list<oper_name_t > appList{memB, memB, memA};

        OperatorSequence seq{{memA, memB}, empty};

        seq.append(appList.cbegin(), appList.cend());
        OperatorSequence seqABA{{memA, memB, memB, memB, memA}, empty};
        EXPECT_EQ(seq, seqABA);
    }

    TEST(Operators_OperatorSequence, Sequence_Append_ABC_initBBA) {
        Context empty{0};
        oper_name_t  memA{1};
        oper_name_t  memB{2};
        oper_name_t  memC{3};

        OperatorSequence seq{{memA, memB, memC}, empty};
        seq.append({memB, memB, memA});
        OperatorSequence seqABAC{{memA, memB, memC, memB, memB, memA}, empty};
        EXPECT_EQ(seq, seqABAC);
    }


    TEST(Operators_OperatorSequence, WithContext_MutexZero) {
        using namespace Moment::Locality;
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


    TEST(Operators_OperatorSequence, Conjugate) {
        using namespace Moment::Algebraic;
        AlgebraicContext context{4, true};
        OperatorSequence seqA{{0, 1, 2, 3}, context};
        OperatorSequence seqB{{3, 2, 1, 0}, context};

        auto conjA = seqA.conjugate();
        EXPECT_EQ(conjA.hash(), seqB.hash());
        ASSERT_EQ(conjA.size(), 4);
        EXPECT_EQ(conjA[0], 3);
        EXPECT_EQ(conjA[1], 2);
        EXPECT_EQ(conjA[2], 1);
        EXPECT_EQ(conjA[3], 0);
        EXPECT_EQ(conjA, seqB);

        auto conjB = seqB.conjugate();
        EXPECT_EQ(conjB.hash(), seqA.hash());
        ASSERT_EQ(conjB.size(), 4);
        EXPECT_EQ(conjB[0], 0);
        EXPECT_EQ(conjB[1], 1);
        EXPECT_EQ(conjB[2], 2);
        EXPECT_EQ(conjB[3], 3);
        EXPECT_EQ(conjB, seqA);

    }

    TEST(Operators_OperatorSequence, Conjugate_Zero) {
        using namespace Moment::Algebraic;
        AlgebraicContext context{4, true};
        auto seqA = OperatorSequence::Zero(context);

        auto conjA = seqA.conjugate();
        EXPECT_EQ(conjA.hash(), seqA.hash());
        ASSERT_EQ(conjA.size(), 0);
        EXPECT_TRUE(conjA.zero());
    }

    TEST(Operators_OperatorSequence, Conjugate_Id) {
        using namespace Moment::Algebraic;
        AlgebraicContext context{4, true};
        auto seqA = OperatorSequence::Identity(context);

        auto conjA = seqA.conjugate();
        EXPECT_EQ(conjA.hash(), seqA.hash());
        ASSERT_EQ(conjA.size(), 0);
        EXPECT_FALSE(conjA.zero());
    }
}
