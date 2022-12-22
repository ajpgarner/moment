/**
 * context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/operator_sequence.h"


namespace Moment::Tests {
    TEST(Operators_Context, Construct_Empty) {
        Context context{0};
        ASSERT_EQ(context.size(), 0);
        ASSERT_TRUE(context.empty());

        EXPECT_EQ(context.size(), 0);
    }

}