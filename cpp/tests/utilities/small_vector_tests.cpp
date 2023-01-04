/**
 * small_vector_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/small_vector.h"

namespace Moment::Tests {

    TEST(Utilities_SmallVector, Construct_Empty) {
        SmallVector<double, 5> empty;
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
        EXPECT_EQ(empty.capacity(), 5);
        EXPECT_FALSE(empty.on_heap());
    }

    TEST(Utilities_SmallVector, Construct_Small) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        EXPECT_EQ(small.size(), 3);
        ASSERT_EQ(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_FALSE(small.on_heap());
    }

    TEST(Utilities_SmallVector, Construct_Large) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 6);
        EXPECT_GE(small.capacity(), 6);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(small[5], 6.0);
        EXPECT_TRUE(small.on_heap());
    }

    TEST(Utilities_SmallVector, PushBack) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 3);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_FALSE(small.on_heap());

        small.push_back(4.0);
        ASSERT_EQ(small.size(), 4);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_FALSE(small.on_heap());

        small.push_back(5.0);
        ASSERT_EQ(small.size(), 5);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_FALSE(small.on_heap());

        small.push_back(6.0);
        ASSERT_EQ(small.size(), 6);
        EXPECT_EQ(small[5], 6.0);
        EXPECT_GE(small.capacity(), 6);
        EXPECT_TRUE(small.on_heap());
    }

    TEST(Utilities_SmallVector, PushBack_HeapToHeap) {
        SmallVector<double, 2> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 3);
        auto old_cap = small.capacity();
        EXPECT_GE(old_cap, 3);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        ASSERT_TRUE(small.on_heap());

        ASSERT_LT(old_cap, 10000) << "Unexpectedly large allocation!";
        for (size_t val = 3; val < old_cap+1; ++val) {
            small.push_back(static_cast<double>(val)+1.0);
        }
        ASSERT_EQ(small.size(), old_cap+1);
        EXPECT_GT(small.capacity(), old_cap);

        for (size_t val = 3; val < old_cap+1; ++val) {
            EXPECT_EQ(small[val], static_cast<double>(val)+1.0);
        }
    }

    TEST(Utilities_SmallVector, Iterator) {
        SmallVector<double, 4> small{1.0, 2.0, 3.0};

        auto iter = small.begin();
        ASSERT_NE(iter, small.end());
        EXPECT_EQ(*iter, 1.0);
        *iter = 10.0;
        EXPECT_EQ(*iter, 10.0);

        ++iter;
        ASSERT_NE(iter, small.end());
        EXPECT_EQ(*iter, 2.0);
        *iter = 20.0;
        EXPECT_EQ(*iter, 20.0);

        ++iter;
        ASSERT_NE(iter, small.end());
        EXPECT_EQ(*iter, 3.0);
        *iter = 30.0;
        EXPECT_EQ(*iter, 30.0);

        ++iter;
        EXPECT_EQ(iter, small.end());

        EXPECT_EQ(small[0], 10.0);
        EXPECT_EQ(small[1], 20.0);
        EXPECT_EQ(small[2], 30.0);

    }

    TEST(Utilities_SmallVector, ConstIterator) {
        SmallVector<double, 4> small{1.0, 2.0, 3.0};

        auto iter = small.cbegin();
        ASSERT_NE(iter, small.cend());
        EXPECT_EQ(*iter, 1.0);

        ++iter;
        ASSERT_NE(iter, small.cend());
        EXPECT_EQ(*iter, 2.0);

        ++iter;
        ASSERT_NE(iter, small.cend());
        EXPECT_EQ(*iter, 3.0);

        ++iter;
        EXPECT_EQ(iter, small.cend());
    }
}