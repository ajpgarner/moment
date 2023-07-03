/**
 * multi_dimensional_offset_index_iterator_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/multi_dimensional_offset_index_iterator.h"

namespace Moment::Tests {

    TEST(Utilities_MDIOffsetIterator, Empty) {
        MultiDimensionalOffsetIndexIterator mdiIter{std::vector<size_t>{}, std::vector<size_t>{}};
        MultiDimensionalOffsetIndexIterator mdiIterEnd{};

        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }



    TEST(Utilities_MDIOffsetIterator, OneDimensional) {
        MultiDimensionalOffsetIndexIterator mdiIter{std::vector<size_t>{2}, std::vector<size_t>{5}}; // 2, 3, 4
        MultiDimensionalOffsetIndexIterator mdiIterEnd{};

        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ(mdiIter.lower_limits().size(), 1);
        ASSERT_EQ(mdiIter.upper_limits().size(), 1);
        ASSERT_EQ(mdiIter.lower_limits()[0], 2);
        ASSERT_EQ(mdiIter.upper_limits()[0], 5);

        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 2);
        EXPECT_EQ(mdiIter.global(), 0);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 3);
        EXPECT_EQ(mdiIter.global(), 1);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 4);
        EXPECT_EQ(mdiIter.global(), 2);

        ++mdiIter;
        EXPECT_TRUE(mdiIter == mdiIterEnd);
    }

    TEST(Utilities_MDIOffsetIterator, TwoDimensional) {

        MultiDimensionalOffsetIndexIterator mdiIter{std::vector<size_t>{2, 0}, std::vector<size_t>{5, 2}};
        MultiDimensionalOffsetIndexIterator mdiIterEnd{};

        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ(mdiIter.lower_limits().size(), 2);
        ASSERT_EQ(mdiIter.upper_limits().size(), 2);
        ASSERT_EQ(mdiIter.lower_limits()[0], 2);
        ASSERT_EQ(mdiIter.lower_limits()[1], 0);
        ASSERT_EQ(mdiIter.upper_limits()[0], 5);
        ASSERT_EQ(mdiIter.upper_limits()[1], 2);

        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ(mdiIter[0], 2);
        EXPECT_EQ(mdiIter[1], 0);
        EXPECT_EQ(mdiIter.global(), 0);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ(mdiIter[0], 2);
        EXPECT_EQ(mdiIter[1], 1);
        EXPECT_EQ(mdiIter.global(), 1);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ(mdiIter[0], 3);
        EXPECT_EQ(mdiIter[1], 0);
        EXPECT_EQ(mdiIter.global(), 2);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ(mdiIter[0], 3);
        EXPECT_EQ(mdiIter[1], 1);
        EXPECT_EQ(mdiIter.global(), 3);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ(mdiIter[0], 4);
        EXPECT_EQ(mdiIter[1], 0);
        EXPECT_EQ(mdiIter.global(), 4);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ(mdiIter[0], 4);
        EXPECT_EQ(mdiIter[1], 1);
        EXPECT_EQ(mdiIter.global(), 5);

        ++mdiIter;
        EXPECT_TRUE(mdiIter == mdiIterEnd);
    }
}