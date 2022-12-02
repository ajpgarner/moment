/**
 * context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/operator_sequence.h"


namespace NPATK::Tests {
    TEST(Operators_Context, Construct_Empty) {
        Context context{0};
        ASSERT_EQ(context.size(), 0);
        ASSERT_TRUE(context.empty());

        EXPECT_EQ(context.size(), 0);
    }

}