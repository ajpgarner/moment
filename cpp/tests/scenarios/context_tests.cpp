/**
 * context_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "dictionary/operator_sequence.h"


namespace Moment::Tests {
    TEST(Scenarios_Context, Construct_Empty) {
        Context context{0};
        ASSERT_EQ(context.size(), 0);
        ASSERT_TRUE(context.empty());

        EXPECT_EQ(context.size(), 0);
    }

}