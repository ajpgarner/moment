/**
 * float_utils_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/float_utils.h"

namespace Moment::Tests {
    TEST(Utilities_FloatUtils, ApproximatelyEqual) {
        float x = 1.0;
        float y = 2.0;
        float z = 2.0;
        EXPECT_FALSE(approximately_equal(x, y));
        EXPECT_FALSE(approximately_equal(x, z));
        EXPECT_FALSE(approximately_equal(y, x));
        EXPECT_FALSE(approximately_equal(z, x));
        EXPECT_TRUE(approximately_equal(x, x));
        EXPECT_TRUE(approximately_equal(y, y));
        EXPECT_TRUE(approximately_equal(z, z));
        EXPECT_TRUE(approximately_equal(y, z));
    }

    TEST(Utilities_FloatUtils, EssentiallyEqual) {
        float x = 1.0;
        float y = 2.0;
        float z = 2.0;
        EXPECT_FALSE(essentially_equal(x, y));
        EXPECT_FALSE(essentially_equal(x, z));
        EXPECT_FALSE(essentially_equal(y, x));
        EXPECT_FALSE(essentially_equal(z, x));
        EXPECT_TRUE(essentially_equal(x, x));
        EXPECT_TRUE(essentially_equal(y, y));
        EXPECT_TRUE(essentially_equal(z, z));
        EXPECT_TRUE(essentially_equal(y, z));
    }

    TEST(Utilities_FloatUtils, ApproximatelyZero) {
        float x = 0.0;
        float y = 1.0;
        float z = 1e-20;
        EXPECT_TRUE(approximately_zero(x));
        EXPECT_FALSE(approximately_zero(y));
        EXPECT_TRUE(approximately_zero(z));
    }


}