/**
 * npa_matrix_Tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/npa_matrix.h"
#include "operators/context.h"

namespace NPATK::Tests {

    TEST(NPAMatrix, Empty) {
        Context context(0, 0); // No parties, no symbols
        ASSERT_EQ(context.size(), 0);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.dimension(), 0);
        auto [rowsLevel0, colsLevel0] = matLevel0.dimensions();
        EXPECT_EQ(rowsLevel0, 0);
        EXPECT_EQ(colsLevel0, 0);
        EXPECT_EQ(matLevel0.UniqueSequences.size(), 2);
        EXPECT_EQ(matLevel0.SymbolMatrix.dimension(), 0);
        EXPECT_EQ(matLevel0.SymbolMatrix.dimensions().first, 0);
        EXPECT_EQ(matLevel0.SymbolMatrix.dimensions().second, 0);

        NPAMatrix matLevel1{context, 1};
        EXPECT_EQ(matLevel1.dimension(), 0);
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        EXPECT_EQ(rowsLevel1, 0);
        EXPECT_EQ(colsLevel1, 0);
        EXPECT_EQ(matLevel1.UniqueSequences.size(), 2);
        EXPECT_EQ(matLevel1.SymbolMatrix.dimension(), 0);
        EXPECT_EQ(matLevel1.SymbolMatrix.dimensions().first, 0);
        EXPECT_EQ(matLevel1.SymbolMatrix.dimensions().second, 0);

        NPAMatrix matLevel5{context, 5};
        EXPECT_EQ(matLevel5.dimension(), 0);
        auto [rowsLevel5, colsLevel5] = matLevel5.dimensions();
        EXPECT_EQ(rowsLevel5, 0);
        EXPECT_EQ(colsLevel5, 0);
        EXPECT_EQ(matLevel5.UniqueSequences.size(), 2);
        EXPECT_EQ(matLevel5.SymbolMatrix.dimension(), 0);
        EXPECT_EQ(matLevel5.SymbolMatrix.dimensions().first, 0);
        EXPECT_EQ(matLevel5.SymbolMatrix.dimensions().second, 0);
    }

    TEST(NPAMatrix, OpSeq_OneElem) {
        Context context{1}; // One party, one symbol
        ASSERT_EQ(context.size(), 1);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.dimension(), 0);
        auto [rowsLevel0, colsLevel0] = matLevel0.dimensions();
        EXPECT_EQ(rowsLevel0, 0);
        EXPECT_EQ(colsLevel0, 0);
        EXPECT_EQ(matLevel0.UniqueSequences.size(), 2);

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.dimension(), 1); // only AA exists
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        ASSERT_EQ(rowsLevel1, 1);
        ASSERT_EQ(colsLevel1, 1);
        EXPECT_EQ(matLevel1[0][0], (OperatorSequence{alice[0], alice[0]}));
        ASSERT_EQ(matLevel1.UniqueSequences.size(), 3);
        const auto& us1_0 = matLevel1.UniqueSequences[2];
        EXPECT_EQ(us1_0.sequence(), (OperatorSequence{alice[0], alice[0]}));
        EXPECT_EQ(us1_0.sequence_conj(), (OperatorSequence{alice[0], alice[0]}));



        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.dimension(), 1); // only AAAA exists
        auto [rowsLevel2, colsLevel2] = matLevel2.dimensions();
        ASSERT_EQ(rowsLevel2, 1);
        ASSERT_EQ(colsLevel2, 1);
        EXPECT_EQ(matLevel2[0][0], (OperatorSequence{alice[0], alice[0], alice[0], alice[0]}));
        EXPECT_EQ(matLevel2.UniqueSequences.size(), 3);
        const auto& us2_0 = matLevel2.UniqueSequences[2];
        EXPECT_EQ(us2_0.sequence(), (OperatorSequence{alice[0], alice[0],alice[0], alice[0]}));
        EXPECT_EQ(us2_0.sequence_conj(), (OperatorSequence{alice[0], alice[0],alice[0], alice[0]}));
    }

    TEST(NPAMatrix, OpSeq_1Party2Opers) {
        Context context{2}; // One party, two symbols
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.dimension(), 0);
        auto [rowsLevel0, colsLevel0] = matLevel0.dimensions();
        EXPECT_EQ(rowsLevel0, 0);
        EXPECT_EQ(colsLevel0, 0);

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.dimension(), 2); // 0, 1
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        ASSERT_EQ(rowsLevel1, 2);
        ASSERT_EQ(colsLevel1, 2);
        EXPECT_EQ(matLevel1[0][0], (OperatorSequence{alice[0], alice[0]}));
        EXPECT_EQ(matLevel1[0][1], (OperatorSequence{alice[0], alice[1]}));
        EXPECT_EQ(matLevel1[1][0], (OperatorSequence{alice[1], alice[0]}));
        EXPECT_EQ(matLevel1[1][1], (OperatorSequence{alice[1], alice[1]}));

        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.dimension(), 4); // 00, 01, 10, 11
        auto [rowsLevel2, colsLevel2] = matLevel2.dimensions();
        ASSERT_EQ(rowsLevel2, 4);
        ASSERT_EQ(colsLevel2, 4);

        EXPECT_EQ(matLevel2[0][0], (OperatorSequence{alice[0], alice[0], alice[0], alice[0]}));
        EXPECT_EQ(matLevel2[0][1], (OperatorSequence{alice[0], alice[0], alice[0], alice[1]}));
        EXPECT_EQ(matLevel2[0][2], (OperatorSequence{alice[0], alice[0], alice[1], alice[0]}));
        EXPECT_EQ(matLevel2[0][3], (OperatorSequence{alice[0], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(matLevel2[1][0], (OperatorSequence{alice[1], alice[0], alice[0], alice[0]}));
        EXPECT_EQ(matLevel2[1][1], (OperatorSequence{alice[1], alice[0], alice[0], alice[1]}));
        EXPECT_EQ(matLevel2[1][2], (OperatorSequence{alice[1], alice[0], alice[1], alice[0]}));
        EXPECT_EQ(matLevel2[1][3], (OperatorSequence{alice[1], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(matLevel2[2][0], (OperatorSequence{alice[0], alice[1], alice[0], alice[0]}));
        EXPECT_EQ(matLevel2[2][1], (OperatorSequence{alice[0], alice[1], alice[0], alice[1]}));
        EXPECT_EQ(matLevel2[2][2], (OperatorSequence{alice[0], alice[1], alice[1], alice[0]}));
        EXPECT_EQ(matLevel2[2][3], (OperatorSequence{alice[0], alice[1], alice[1], alice[1]}));
        EXPECT_EQ(matLevel2[3][0], (OperatorSequence{alice[1], alice[1], alice[0], alice[0]}));
        EXPECT_EQ(matLevel2[3][1], (OperatorSequence{alice[1], alice[1], alice[0], alice[1]}));
        EXPECT_EQ(matLevel2[3][2], (OperatorSequence{alice[1], alice[1], alice[1], alice[0]}));
        EXPECT_EQ(matLevel2[3][3], (OperatorSequence{alice[1], alice[1], alice[1], alice[1]}));
    };

    TEST(NPAMatrix, OpSeq_2Party1Opers) {
        Context context{1, 1}; // Two parties, each with one operator
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.dimension(), 0);
        auto [rowsLevel0, colsLevel0] = matLevel0.dimensions();
        EXPECT_EQ(rowsLevel0, 0);
        EXPECT_EQ(colsLevel0, 0);


        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.dimension(), 2); // A, B
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        ASSERT_EQ(rowsLevel1, 2);
        ASSERT_EQ(colsLevel1, 2);
        EXPECT_EQ(matLevel1[0][0], (OperatorSequence{alice[0], alice[0]}));
        EXPECT_EQ(matLevel1[0][1], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel1[1][0], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel1[1][1], (OperatorSequence{bob[0], bob[0]}));

        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.dimension(), 3); // AA, AB, BB
        auto [rowsLevel2, colsLevel2] = matLevel2.dimensions();
        ASSERT_EQ(rowsLevel2, 3);
        ASSERT_EQ(colsLevel2, 3);

        EXPECT_EQ(matLevel2[0][0], (OperatorSequence{alice[0], alice[0], alice[0], alice[0]}));
        EXPECT_EQ(matLevel2[0][1], (OperatorSequence{alice[0], alice[0], alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[0][2], (OperatorSequence{alice[0], alice[0], bob[0], bob[0]}));
        EXPECT_EQ(matLevel2[1][0], (OperatorSequence{alice[0], alice[0], alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[1][1], (OperatorSequence{alice[0], alice[0], bob[0], bob[0]}));
        EXPECT_EQ(matLevel2[1][2], (OperatorSequence{alice[0], bob[0], bob[0], bob[0]}));
        EXPECT_EQ(matLevel2[2][0], (OperatorSequence{alice[0], alice[0], bob[0], bob[0]}));
        EXPECT_EQ(matLevel2[2][1], (OperatorSequence{alice[0], bob[0], bob[0], bob[0]}));
        EXPECT_EQ(matLevel2[2][2], (OperatorSequence{bob[0], bob[0], bob[0], bob[0]}));
    }

    TEST(NPAMatrix, OpSeq_2Party1OpersIdem) {
        Context context(2, 1, Operator::Flags::Idempotent); // Two party, one operator
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.dimension(), 0);
        auto [rowsLevel0, colsLevel0] = matLevel0.dimensions();
        EXPECT_EQ(rowsLevel0, 0);
        EXPECT_EQ(colsLevel0, 0);

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.dimension(), 2); // A, B
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        ASSERT_EQ(rowsLevel1, 2);
        ASSERT_EQ(colsLevel1, 2);
        EXPECT_EQ(matLevel1[0][0], (OperatorSequence{alice[0]}));
        EXPECT_EQ(matLevel1[0][1], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel1[1][0], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel1[1][1], (OperatorSequence{bob[0]}));


        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.dimension(), 3); // A, B, AB
        auto [rowsLevel2, colsLevel2] = matLevel2.dimensions();
        ASSERT_EQ(rowsLevel2, 3);
        ASSERT_EQ(colsLevel2, 3);

        EXPECT_EQ(matLevel2[0][0], (OperatorSequence{alice[0]}));
        EXPECT_EQ(matLevel2[0][1], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[0][2], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[1][0], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[1][1], (OperatorSequence{bob[0]}));
        EXPECT_EQ(matLevel2[1][2], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[2][0], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[2][1], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(matLevel2[2][2], (OperatorSequence{alice[0], bob[0]}));

    }

    TEST(NPAMatrix, Unique_OneElem) {
        Context context{1}; // One party, one symbol
        ASSERT_EQ(context.size(), 1);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);

        NPAMatrix matLevel0{context, 0};
        ASSERT_EQ(matLevel0.UniqueSequences.size(), 2);
        const auto& us0_0 = matLevel0.UniqueSequences[0];
        const auto& us0_1 = matLevel0.UniqueSequences[1];
        EXPECT_EQ(us0_0.sequence(), OperatorSequence::Zero(&context));
        EXPECT_EQ(us0_1.sequence(), OperatorSequence::Identity(&context));

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.UniqueSequences.size(), 3);
        const auto& us1_0 = matLevel1.UniqueSequences[0];
        const auto& us1_1 = matLevel1.UniqueSequences[1];
        const auto& us1_2 = matLevel1.UniqueSequences[2];
        EXPECT_EQ(us1_0.sequence(), OperatorSequence::Zero(&context));
        EXPECT_EQ(us1_1.sequence(), OperatorSequence::Identity(&context));
        EXPECT_EQ(us1_2.sequence(), (OperatorSequence{alice[0], alice[0]}));
        EXPECT_EQ(us1_2.sequence_conj(), (OperatorSequence{alice[0], alice[0]}));

        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.UniqueSequences.size(), 3);
        const auto& us2_0 = matLevel2.UniqueSequences[0];
        const auto& us2_1 = matLevel2.UniqueSequences[1];
        const auto& us2_2 = matLevel2.UniqueSequences[2];
        EXPECT_EQ(us2_2.sequence(), (OperatorSequence{alice[0], alice[0],alice[0], alice[0]}));
        EXPECT_EQ(us2_2.sequence_conj(), (OperatorSequence{alice[0], alice[0],alice[0], alice[0]}));
    }

    TEST(NPAMatrix, Unique_2Party1Opers) {
        Context context{1, 1}; // Two parties, each with one operator
        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.UniqueSequences.size(), 2);

        NPAMatrix matLevel1{context, 1};
        EXPECT_EQ(matLevel1.UniqueSequences.size(), 5);

        NPAMatrix matLevel2{context, 2};
        EXPECT_EQ(matLevel2.UniqueSequences.size(), 7); // because aabb and abab->aabb are the same...
    }

    TEST(NPAMatrix, Unique_2Party1OpersIdem) {
        Context context(2, 1, Operator::Flags::Idempotent); // Two party, one operator
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        const auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 1);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.UniqueSequences.size(), 2);

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.UniqueSequences.size(), 5);
        const auto& us1_0 = matLevel1.UniqueSequences[2];
        EXPECT_EQ(us1_0.sequence(), (OperatorSequence{alice[0]}));
        EXPECT_TRUE(us1_0.is_hermitian());

        const auto& us1_1 = matLevel1.UniqueSequences[3];
        EXPECT_EQ(us1_1.sequence(), (OperatorSequence{bob[0]}));
        EXPECT_TRUE(us1_1.is_hermitian());

        const auto& us1_2 = matLevel1.UniqueSequences[4];
        EXPECT_EQ(us1_2.sequence(), (OperatorSequence{alice[0], bob[0]}));
        EXPECT_TRUE(us1_2.is_hermitian());

        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.UniqueSequences.size(), 5);

        const auto& us2_0 = matLevel2.UniqueSequences[2];
        EXPECT_EQ(us2_0.sequence(), (OperatorSequence{alice[0]}));
        EXPECT_TRUE(us2_0.is_hermitian());

        const auto& us2_1 = matLevel2.UniqueSequences[3];
        EXPECT_EQ(us2_1.sequence(), (OperatorSequence{bob[0]}));
        EXPECT_TRUE(us2_1.is_hermitian());

        const auto& us2_2 = matLevel2.UniqueSequences[4];
        EXPECT_EQ(us2_2.sequence(), (OperatorSequence{alice[0], bob[0]}));
        EXPECT_TRUE(us2_2.is_hermitian());

    }

    TEST(NPAMatrix, Unique_1Party2Opers) {
        Context context{2}; // One party, two symbols
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        NPAMatrix matLevel0{context, 0};
        ASSERT_EQ(matLevel0.UniqueSequences.size(), 2);
        const auto& us0_0 = matLevel0.UniqueSequences[0];
        const auto& us0_1 = matLevel0.UniqueSequences[1];
        EXPECT_EQ(us0_0.sequence(), OperatorSequence::Zero(&context));
        EXPECT_EQ(us0_1.sequence(), OperatorSequence::Identity(&context));


        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.UniqueSequences.size(), 5); // 1 less than 4, because 10 = 01*
        const auto& us1_0 = matLevel1.UniqueSequences[0];
        const auto& us1_1 = matLevel1.UniqueSequences[1];
        const auto& us1_2 = matLevel1.UniqueSequences[2];
        const auto& us1_3 = matLevel1.UniqueSequences[3];
        const auto& us1_4 = matLevel1.UniqueSequences[4];
        EXPECT_EQ(us1_0.sequence(), OperatorSequence::Zero(&context));
        EXPECT_EQ(us1_1.sequence(), OperatorSequence::Identity(&context));
        EXPECT_EQ(us1_2.sequence(), (OperatorSequence{alice[0], alice[0]}));
        EXPECT_TRUE(us1_2.is_hermitian());
        EXPECT_EQ(us1_3.sequence(), (OperatorSequence{alice[0], alice[1]}));
        EXPECT_EQ(us1_3.sequence_conj(), (OperatorSequence{alice[1], alice[0]}));
        EXPECT_FALSE(us1_3.is_hermitian());
        EXPECT_EQ(us1_4.sequence(), (OperatorSequence{alice[1], alice[1]}));
        EXPECT_TRUE(us1_4.is_hermitian());


        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.UniqueSequences.size(), 12); // Subject to complex conjugation, every elem is otherwise unique.

        struct test_seq {
            OperatorSequence fwd;
            OperatorSequence rev;
            bool herm;
        };

        std::vector<test_seq> references{
                {OperatorSequence::Zero(&context), OperatorSequence::Zero(&context), true},
                {OperatorSequence::Identity(&context), OperatorSequence::Identity(&context), true},
                {OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[0], alice[0], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[0], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[0], alice[1], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[1], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[0], alice[1], alice[1]}, &context),
                        OperatorSequence({alice[1], alice[1], alice[0], alice[0]}, &context), false},
                {OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, &context),
                        OperatorSequence({alice[0], alice[1], alice[1], alice[0]}, &context), true},
                {OperatorSequence({alice[0], alice[1], alice[1], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[1], alice[1], alice[0]} , &context), false},
                {OperatorSequence({alice[1], alice[0], alice[0], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[0], alice[0], alice[1]} , &context), true},
                {OperatorSequence({alice[1], alice[0], alice[1], alice[0]} , &context),
                        OperatorSequence({alice[0], alice[1], alice[0], alice[1]} , &context), false},
                {OperatorSequence({alice[1], alice[0], alice[1], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[1], alice[0], alice[1]} , &context), false},
                {OperatorSequence({alice[1], alice[1], alice[1], alice[1]} , &context),
                        OperatorSequence({alice[1], alice[1], alice[1], alice[1]} , &context), true}
        };

        size_t count = 0;
        for (const auto& refs : references) {
            const auto& val = matLevel2.UniqueSequences[count];
            EXPECT_EQ(val.sequence(), refs.fwd);
            ASSERT_EQ(val.is_hermitian(), refs.herm);
            if (!refs.herm) {
                EXPECT_EQ(val.sequence_conj(), refs.rev);
            }
            ++count;
        }

    };

    TEST(NPAMatrix, Where_1Party2Opers) {
        Context context{2}; // One parties with two operators.
        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.UniqueSequences.size(), 12);

        auto ptr_a0a0a0a0 = matLevel2.where(OperatorSequence{alice[0], alice[0], alice[0], alice[0]});
        ASSERT_NE(ptr_a0a0a0a0, nullptr);
        EXPECT_EQ(ptr_a0a0a0a0->sequence(), (OperatorSequence{alice[0], alice[0], alice[0], alice[0]}));

        const auto& us2_5 = matLevel2.UniqueSequences[5];
        EXPECT_EQ(us2_5.sequence(), (OperatorSequence{alice[0], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(us2_5.sequence_conj(), (OperatorSequence{alice[1], alice[1], alice[0], alice[0]}));
        EXPECT_FALSE(us2_5.is_hermitian());

        auto ptr_a0a0a1a1 = matLevel2.where(OperatorSequence{alice[0], alice[0], alice[1], alice[1]});
        auto ptr_a1a1a0a0 = matLevel2.where(OperatorSequence{alice[1], alice[1], alice[0], alice[0]});
        ASSERT_NE(ptr_a0a0a1a1, nullptr);
        ASSERT_NE(ptr_a1a1a0a0, nullptr);
        EXPECT_EQ(ptr_a0a0a1a1, ptr_a1a1a0a0);

        EXPECT_EQ(ptr_a0a0a1a1->sequence(), (OperatorSequence{alice[0], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(ptr_a1a1a0a0->sequence(), (OperatorSequence{alice[0], alice[0], alice[1], alice[1]}));
        EXPECT_EQ(ptr_a0a0a1a1->sequence_conj(), (OperatorSequence{alice[1], alice[1], alice[0], alice[0]}));
        EXPECT_EQ(ptr_a1a1a0a0->sequence_conj(), (OperatorSequence{alice[1], alice[1], alice[0], alice[0]}));

        auto ptr_a0a0a0a0a0 = matLevel2.where(OperatorSequence{alice[0], alice[0], alice[0], alice[0], alice[0]});
        EXPECT_EQ(ptr_a0a0a0a0a0, nullptr);
    }


    TEST(NPAMatrix, Symbol_OneElem) {
        Context context{1}; // One party, one symbol

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.SymbolMatrix.dimension(), 1);
        ASSERT_EQ(matLevel1.SymbolMatrix.dimensions().first, 1);
        ASSERT_EQ(matLevel1.SymbolMatrix.dimensions().second, 1);
        EXPECT_EQ(matLevel1.SymbolMatrix[0][0], SymbolExpression{2});

        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.SymbolMatrix.dimension(), 1);
        ASSERT_EQ(matLevel2.SymbolMatrix.dimensions().first, 1);
        ASSERT_EQ(matLevel2.SymbolMatrix.dimensions().second, 1);
        EXPECT_EQ(matLevel2.SymbolMatrix[0][0], SymbolExpression{2});
    }


}