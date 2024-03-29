/**
 * localizing_matrix_tests.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"

#include "matrix_system/matrix_system.h"
#include "matrix/operator_matrix/localizing_matrix.h"

#include "compare_os_matrix.h"

namespace Moment::Tests {
    TEST(Matrix_LocalizingMatrix, OpSeq_OneElem) {
        MatrixSystem system{std::make_unique<Context>(1)}; //One symbol
        const auto& context = system.Context();

        oper_name_t theOp = 0;

        ASSERT_EQ(context.size(), 1);

        OperatorSequence genWord{{theOp}, context};
        auto [id, matLevel0] = system.LocalizingMatrix.create(LocalizingMatrixIndex{0, genWord});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel0), nullptr);
        const auto& lm0 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel0);

        EXPECT_EQ(lm0.Index.Level, 0);
        EXPECT_EQ(lm0.Index.Word, genWord);
        compare_lm_os_matrix(matLevel0, 1, {OperatorSequence({theOp}, context)});


        auto [id1, matLevel1] = system.LocalizingMatrix.create(LocalizingMatrixIndex{1, genWord});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel1), nullptr);
        const auto& lm1 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel1);
        EXPECT_EQ(lm1.Index.Level, 1);
        EXPECT_EQ(lm1.Index.Word, genWord);
        compare_lm_os_matrix(matLevel1, 2, {OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context)});


        auto [id2, matLevel2] = system.LocalizingMatrix.create(LocalizingMatrixIndex{2, genWord});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel2), nullptr);
        const auto& lm2 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel2);
        EXPECT_EQ(lm2.Index.Level, 2);
        EXPECT_EQ(lm2.Index.Word, genWord);
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

    TEST(Matrix_LocalizingMatrix, OpSeq_TwoElem) {
        MatrixSystem system{std::make_unique<Context>(2)}; // One party, two symbols
        const auto& context = system.Context();

        ASSERT_EQ(context.size(), 2);
        const auto op0 = 0; //context[0];
        const auto op1 = 1; //context[1];

        OperatorSequence genWord0{{op0}, context};
        OperatorSequence genWord1{{op1}, context};


        auto [id00, matLevel00] = system.LocalizingMatrix.create(LocalizingMatrixIndex{0, genWord0});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel00), nullptr);
        const auto& lm00 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel00);
        EXPECT_EQ(lm00.Index.Level, 0);
        EXPECT_EQ(lm00.Index.Word, genWord0);
        compare_lm_os_matrix(matLevel00, 1, {OperatorSequence({op0}, context)});

        auto [id01, matLevel01] = system.LocalizingMatrix.create(LocalizingMatrixIndex{0, genWord1});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel01), nullptr);
        const auto& lm01 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel01);
        EXPECT_EQ(lm01.Index.Level, 0);
        EXPECT_EQ(lm01.Index.Word, genWord1);
        compare_lm_os_matrix(matLevel01, 1, {OperatorSequence({op1}, context)});


        auto [id10, matLevel10] = system.LocalizingMatrix.create(LocalizingMatrixIndex{1, genWord0});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel10), nullptr);
        const auto& lm10 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel10);
        EXPECT_EQ(lm10.Index.Level, 1);
        EXPECT_EQ(lm10.Index.Word, genWord0);
        compare_lm_os_matrix(matLevel10, 3, {OperatorSequence({op0}, context),
                                            OperatorSequence({op0, op0}, context),
                                            OperatorSequence({op0, op1}, context),
                                            OperatorSequence({op0, op0}, context),
                                            OperatorSequence({op0, op0, op0}, context),
                                            OperatorSequence({op0, op0, op1}, context),
                                            OperatorSequence({op1, op0}, context),
                                            OperatorSequence({op1, op0, op0}, context),
                                            OperatorSequence({op1, op0, op1}, context)});

        auto [id11, matLevel11] = system.LocalizingMatrix.create(LocalizingMatrixIndex{1, genWord1});
        ASSERT_NE(LocalizingMatrix::to_operator_matrix_ptr(matLevel11), nullptr);
        const auto& lm11 = *LocalizingMatrix::to_operator_matrix_ptr(matLevel11);
        EXPECT_EQ(lm11.Index.Level, 1);
        EXPECT_EQ(lm11.Index.Word, genWord1);
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