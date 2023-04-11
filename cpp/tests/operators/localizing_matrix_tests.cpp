/**
 * localizing_matrix_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"

#include "matrix_system.h"
#include "matrix/localizing_matrix.h"

#include "compare_os_matrix.h"

namespace Moment::Tests {
    TEST(Operators_LocalizingMatrix, OpSeq_OneElem) {
        MatrixSystem system{std::make_unique<Context>(1)}; //One symbol
        const auto& context = system.Context();

        oper_name_t theOp = 0; // context[0];

        ASSERT_EQ(context.size(), 1);

        OperatorSequence genWord{{theOp}, context};

        auto [id, matLevel0] = system.create_localizing_matrix(LocalizingMatrixIndex{0, genWord});

        EXPECT_EQ(matLevel0.Level(), 0);
        EXPECT_EQ(matLevel0.Word(), genWord);
        compare_lm_os_matrix(matLevel0, 1, {OperatorSequence({theOp}, context)});


        auto [id1, matLevel1] = system.create_localizing_matrix(LocalizingMatrixIndex{1, genWord});
        EXPECT_EQ(matLevel1.Level(), 1);
        EXPECT_EQ(matLevel1.Word(), genWord);
        compare_lm_os_matrix(matLevel1, 2, {OperatorSequence({theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp}, context),
                                         OperatorSequence({theOp, theOp, theOp}, context)});


        auto [id2, matLevel2] = system.create_localizing_matrix(LocalizingMatrixIndex{2, genWord});
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

    TEST(Operators_LocalizingMatrix, OpSeq_TwoElem) {
        MatrixSystem system{std::make_unique<Context>(2)}; // One party, two symbols
        const auto& context = system.Context();

        ASSERT_EQ(context.size(), 2);
        const auto op0 = 0; //context[0];
        const auto op1 = 1; //context[1];

        OperatorSequence genWord0{{op0}, context};
        OperatorSequence genWord1{{op1}, context};


        auto [id00, matLevel00] = system.create_localizing_matrix(LocalizingMatrixIndex{0, genWord0});
        EXPECT_EQ(matLevel00.Level(), 0);
        EXPECT_EQ(matLevel00.Word(), genWord0);
        compare_lm_os_matrix(matLevel00, 1, {OperatorSequence({op0}, context)});

        auto [id01, matLevel01] = system.create_localizing_matrix(LocalizingMatrixIndex{0, genWord1});
        EXPECT_EQ(matLevel01.Level(), 0);
        EXPECT_EQ(matLevel01.Word(), genWord1);
        compare_lm_os_matrix(matLevel01, 1, {OperatorSequence({op1}, context)});


        auto [id10, matLevel10] = system.create_localizing_matrix(LocalizingMatrixIndex{1, genWord0});
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

        auto [id11, matLevel11] = system.create_localizing_matrix(LocalizingMatrixIndex{1, genWord1});
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