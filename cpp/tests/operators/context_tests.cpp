/**
 * context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/operator_sequence.h"


namespace NPATK::Tests {
    TEST(Context, Construct_Empty) {
        Context context{0};
        ASSERT_EQ(context.size(), 0);
        ASSERT_TRUE(context.empty());

        auto iter_begin = context.begin();
        auto iter_end = context.end();
        EXPECT_EQ(iter_begin, iter_end);

        EXPECT_EQ(context.size(), 0);
    }

}