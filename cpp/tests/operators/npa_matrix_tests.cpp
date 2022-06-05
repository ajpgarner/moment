/**
 * npa_matrix_Tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/npa_matrix.h"
#include "operators/context.h"

namespace NPATK::Tests {

    TEST(NPAMatrix, Construct_Empty) {
        Context context(0, 0); // No parties, no symbols
        ASSERT_EQ(context.size(), 0);

        NPAMatrix matLevel0{context, 0};
        EXPECT_EQ(matLevel0.dimension(), 0);
        auto [rowsLevel0, colsLevel0] = matLevel0.dimensions();
        EXPECT_EQ(rowsLevel0, 0);
        EXPECT_EQ(colsLevel0, 0);

        NPAMatrix matLevel1{context, 1};
        EXPECT_EQ(matLevel1.dimension(), 0);
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        EXPECT_EQ(rowsLevel1, 0);
        EXPECT_EQ(colsLevel1, 0);

        NPAMatrix matLevel5{context, 5};
        EXPECT_EQ(matLevel5.dimension(), 0);
        auto [rowsLevel5, colsLevel5] = matLevel5.dimensions();
        EXPECT_EQ(rowsLevel5, 0);
        EXPECT_EQ(colsLevel5, 0);
    }



    TEST(NPAMatrix, Construct_OneElem) {
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

        NPAMatrix matLevel1{context, 1};
        ASSERT_EQ(matLevel1.dimension(), 1); // only AA exists
        auto [rowsLevel1, colsLevel1] = matLevel1.dimensions();
        ASSERT_EQ(rowsLevel1, 1);
        ASSERT_EQ(colsLevel1, 1);
        EXPECT_EQ(matLevel1[0][0], (OperatorSequence{alice[0], alice[0]}));


        NPAMatrix matLevel2{context, 2};
        ASSERT_EQ(matLevel2.dimension(), 1); // only AAAA exists
        auto [rowsLevel2, colsLevel2] = matLevel2.dimensions();
        ASSERT_EQ(rowsLevel2, 1);
        ASSERT_EQ(colsLevel2, 1);
        EXPECT_EQ(matLevel2[0][0], (OperatorSequence{alice[0], alice[0], alice[0], alice[0]}));
    }

    TEST(NPAMatrix, Construct_1Party2Opers) {
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

    }

    TEST(NPAMatrix, Construct_2Party1Opers) {
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

    TEST(NPAMatrix, Construct_2Party1Opers_Idem) {
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

}