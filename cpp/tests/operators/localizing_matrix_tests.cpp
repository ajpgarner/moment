/**
 * localizing_matrix_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/matrix/localizing_matrix.h"
#include "operators/matrix/matrix_system.h"


#include "compare_os_matrix.h"

namespace NPATK::Tests {
    TEST(LocalizingMatrix, OpSeq_OneElem) {
        MatrixSystem system{std::make_unique<Context>(1)}; //One symbol
        auto& context = system.Context();

        auto& theOp = context[0];

        ASSERT_EQ(context.size(), 1);

        OperatorSequence genWord{{theOp}, context};

        auto& matLevel0 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 0, genWord});

        EXPECT_EQ(matLevel0.Level(), 0);
        EXPECT_EQ(matLevel0.Word(), genWord);
        compare_lm_os_matrix(matLevel0, 1, {OperatorSequence({theOp}, context)});


        auto& matLevel1 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 1, genWord});
        EXPECT_EQ(matLevel1.Level(), 1);
        EXPECT_EQ(matLevel1.Word(), genWord);
        compare_lm_os_matrix(matLevel1, 2, {OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context)});


        auto& matLevel2 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 2, genWord});
        EXPECT_EQ(matLevel2.Level(), 2);
        EXPECT_EQ(matLevel2.Word(), genWord);
        compare_lm_os_matrix(matLevel2, 3, {OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp, theOp, theOp}, context)});
    }

    TEST(LocalizingMatrix, OpSeq_TwoElem) {
        MatrixSystem system{std::make_unique<Context>(2)}; // One party, two symbols
        auto& context = system.Context();

        ASSERT_EQ(context.size(), 2);
        const auto& op0 = context[0];
        const auto& op1 = context[1];

        OperatorSequence genWord0{{op0}, context};
        OperatorSequence genWord1{{op1}, context};


        auto& matLevel00 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 0, genWord0});
        EXPECT_EQ(matLevel00.Level(), 0);
        EXPECT_EQ(matLevel00.Word(), genWord0);
        compare_lm_os_matrix(matLevel00, 1, {OperatorSequence({op0}, context)});

        auto& matLevel01 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 0, genWord1});
        EXPECT_EQ(matLevel01.Level(), 0);
        EXPECT_EQ(matLevel01.Word(), genWord1);
        compare_lm_os_matrix(matLevel01, 1, {OperatorSequence({op1}, context)});


        auto& matLevel10 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 1, genWord0});
        EXPECT_EQ(matLevel10.Level(), 1);
        EXPECT_EQ(matLevel10.Word(), genWord0);
        compare_lm_os_matrix(matLevel10, 3, {OperatorSequence({op0}, context),
                                            OperatorSequence({op0, op0}, context),
                                            OperatorSequence({op0, op1}, context),
                                            OperatorSequence({op0, op0}, context),
                                            OperatorSequence({op0, op0, op0}, context),
                                            OperatorSequence({op0, op0, op1}, context),
                                            OperatorSequence({op1, op0}, context),
                                            OperatorSequence({op1, op0, op0}, context),
                                            OperatorSequence({op1, op0, op1}, context)});

        auto& matLevel11 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 1, genWord1});
        EXPECT_EQ(matLevel11.Level(), 1);
        EXPECT_EQ(matLevel11.Word(), genWord1);
        compare_lm_os_matrix(matLevel11, 3, {OperatorSequence({op1}, context),
                                             OperatorSequence({op1, op0}, context),
                                             OperatorSequence({op1, op1}, context),
                                             OperatorSequence({op0, op1}, context),
                                             OperatorSequence({op0, op1, op0}, context),
                                             OperatorSequence({op0, op1, op1}, context),
                                             OperatorSequence({op1, op1}, context),
                                             OperatorSequence({op1, op1, op0}, context),
                                             OperatorSequence({op1, op1, op1}, context)});

    }
}