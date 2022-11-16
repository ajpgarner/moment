/**
 * inflation_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

namespace NPATK::Tests {

    TEST(InflationContext, Empty) {
        InflationContext ic{CausalNetwork{{}, {}}, 0};
        EXPECT_EQ(ic.size(), 0);
    }

}
