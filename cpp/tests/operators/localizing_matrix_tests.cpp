/**
 * localizing_matrix_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/matrix/matrix_system.h"
#include "operators/matrix/localizing_matrix.h"

#include "compare_os_matrix.h"

namespace NPATK::Tests {
    TEST(LocalizingMatrix, OpSeq_OneElem) {
        MatrixSystem system{std::make_unique<Context>(std::initializer_list<oper_name_t>{1})}; // One party, one symbol
        auto& context = system.Context();

        ASSERT_EQ(context.size(), 1);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 1);

        OperatorSequence genWord{{alice[0]}, &context};

        auto& matLevel0 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 0, genWord});

        EXPECT_EQ(matLevel0.Level(), 0);
        EXPECT_EQ(matLevel0.Word(), genWord);
        compare_lm_os_matrix(matLevel0, 1, {OperatorSequence({alice[0]}, &context)});


        auto& matLevel1 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 1, genWord});
        EXPECT_EQ(matLevel1.Level(), 1);
        EXPECT_EQ(matLevel1.Word(), genWord);
        compare_lm_os_matrix(matLevel1, 2, {OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context)});


        auto& matLevel2 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 2, genWord});
        EXPECT_EQ(matLevel2.Level(), 2);
        EXPECT_EQ(matLevel2.Word(), genWord);
        compare_lm_os_matrix(matLevel2, 3, {OperatorSequence({alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0]}, &context),
                                         OperatorSequence({alice[0], alice[0], alice[0], alice[0], alice[0]}, &context)});
    }

    TEST(LocalizingMatrix, OpSeq_TwoElem) {
        MatrixSystem system{std::make_unique<Context>(std::initializer_list<oper_name_t>{2})}; // One party, two symbols
        auto& context = system.Context();

        ASSERT_EQ(context.size(), 2);
        ASSERT_EQ(context.Parties.size(), 1);
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        OperatorSequence genWord0{{alice[0]}, &context};
        OperatorSequence genWord1{{alice[1]}, &context};


        auto& matLevel00 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 0, genWord0});
        EXPECT_EQ(matLevel00.Level(), 0);
        EXPECT_EQ(matLevel00.Word(), genWord0);
        compare_lm_os_matrix(matLevel00, 1, {OperatorSequence({alice[0]}, &context)});

        auto& matLevel01 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 0, genWord1});
        EXPECT_EQ(matLevel01.Level(), 0);
        EXPECT_EQ(matLevel01.Word(), genWord1);
        compare_lm_os_matrix(matLevel01, 1, {OperatorSequence({alice[1]}, &context)});


        auto& matLevel10 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 1, genWord0});
        EXPECT_EQ(matLevel10.Level(), 1);
        EXPECT_EQ(matLevel10.Word(), genWord0);
        compare_lm_os_matrix(matLevel10, 3, {OperatorSequence({alice[0]}, &context),
                                            OperatorSequence({alice[0], alice[0]}, &context),
                                            OperatorSequence({alice[0], alice[1]}, &context),
                                            OperatorSequence({alice[0], alice[0]}, &context),
                                            OperatorSequence({alice[0], alice[0], alice[0]}, &context),
                                            OperatorSequence({alice[0], alice[0], alice[1]}, &context),
                                            OperatorSequence({alice[1], alice[0]}, &context),
                                            OperatorSequence({alice[1], alice[0], alice[0]}, &context),
                                            OperatorSequence({alice[1], alice[0], alice[1]}, &context)});

        auto& matLevel11 = system.CreateLocalizingMatrix(LocalizingMatrixIndex{context, 1, genWord1});
        EXPECT_EQ(matLevel11.Level(), 1);
        EXPECT_EQ(matLevel11.Word(), genWord1);
        compare_lm_os_matrix(matLevel11, 3, {OperatorSequence({alice[1]}, &context),
                                             OperatorSequence({alice[1], alice[0]}, &context),
                                             OperatorSequence({alice[1], alice[1]}, &context),
                                             OperatorSequence({alice[0], alice[1]}, &context),
                                             OperatorSequence({alice[0], alice[1], alice[0]}, &context),
                                             OperatorSequence({alice[0], alice[1], alice[1]}, &context),
                                             OperatorSequence({alice[1], alice[1]}, &context),
                                             OperatorSequence({alice[1], alice[1], alice[0]}, &context),
                                             OperatorSequence({alice[1], alice[1], alice[1]}, &context)});

    }
}