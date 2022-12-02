/**
 * ordered_combination_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/ordered_permutation.h"

namespace NPATK::Tests {

    TEST(Utilities_OrderedPermutation, Gen2C1) {
        OrderedPermutationIterator<int> opi{2, 1};
        OrderedPermutationIterator<int> opi_end{2, 1, true};

        ASSERT_NE(opi, opi_end);
        ASSERT_EQ(opi->size(), 1);

        EXPECT_EQ(opi[0], 0);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        ++opi;

        ASSERT_EQ(opi, opi_end);
    }

    TEST(Utilities_OrderedPermutation, Gen2C2) {
        OrderedPermutationIterator<int> opi{2, 2};
        OrderedPermutationIterator<int> opi_end{2, 2, true};

        ASSERT_NE(opi, opi_end);
        ASSERT_EQ(opi->size(), 2);

        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 1);
        ++opi;

        ASSERT_EQ(opi, opi_end);
    }

    TEST(Utilities_OrderedPermutation, Gen2C3) {
        OrderedPermutationIterator<int> opi{2, 3};
        OrderedPermutationIterator<int> opi_end{2, 3, true};

        ASSERT_NE(opi, opi_end);
        ASSERT_EQ(opi->size(), 3);

        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        EXPECT_EQ(opi[2], 0);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        EXPECT_EQ(opi[2], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 1);
        EXPECT_EQ(opi[2], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 1);
        EXPECT_EQ(opi[2], 1);
        ++opi;

        ASSERT_EQ(opi, opi_end);
    }

    TEST(Utilities_OrderedPermutation, Gen3C2) {
        OrderedPermutationIterator<int> opi{3, 2};
        OrderedPermutationIterator<int> opi_end{3, 2, true};

        ASSERT_NE(opi, opi_end);
        ASSERT_EQ(opi->size(), 2);

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 2);
        EXPECT_EQ(opi[1], 2);
        ++opi;

        ASSERT_EQ(opi, opi_end);
    }

    TEST(Utilities_OrderedPermutation, Gen3C3) {
        OrderedPermutationIterator<int> opi{3, 3};
        OrderedPermutationIterator<int> opi_end{3, 3, true};

        ASSERT_NE(opi, opi_end);
        ASSERT_EQ(opi->size(), 3);

        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        EXPECT_EQ(opi[2], 0);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        EXPECT_EQ(opi[2], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 0);
        EXPECT_EQ(opi[2], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 1);
        EXPECT_EQ(opi[2], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 1);
        EXPECT_EQ(opi[2], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 0);
        EXPECT_EQ(opi[1], 2);
        EXPECT_EQ(opi[2], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 1);
        EXPECT_EQ(opi[2], 1);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 1);
        EXPECT_EQ(opi[2], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 1);
        EXPECT_EQ(opi[1], 2);
        EXPECT_EQ(opi[2], 2);
        ++opi;

        ASSERT_NE(opi, opi_end);
        EXPECT_EQ(opi[0], 2);
        EXPECT_EQ(opi[1], 2);
        EXPECT_EQ(opi[2], 2);
        ++opi;


        ASSERT_EQ(opi, opi_end);
    }

}
