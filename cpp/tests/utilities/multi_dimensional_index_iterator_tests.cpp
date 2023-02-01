/**
 * multi_dimensional_index_iterator_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "utilities/multi_dimensional_index_iterator.h"

namespace Moment::Tests {

    TEST(Utilities_MDIIterator, Empty) {
        MultiDimensionalIndexIterator mdiIter{{}};
        MultiDimensionalIndexIterator mdiIterEnd{{}, true};

        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_MDIIterator, OneDimensional) {
        MultiDimensionalIndexIterator mdiIter{{4}};
        MultiDimensionalIndexIterator mdiIterEnd{{}, true};

        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ(mdiIter.limits().size(), 1);
        ASSERT_EQ(mdiIter.limits()[0], 4);

        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 0);
        EXPECT_EQ(mdiIter.global(), 0);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 1);
        EXPECT_EQ(mdiIter.global(), 1);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 2);
        EXPECT_EQ(mdiIter.global(), 2);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 1);
        EXPECT_EQ((*mdiIter)[0], 3);
        EXPECT_EQ(mdiIter.global(), 3);

        ++mdiIter;
        EXPECT_TRUE(mdiIter == mdiIterEnd);
    }

    TEST(Utilities_MDIIterator, TwoDimensional) {
        MultiDimensionalIndexIterator mdiIter{{3, 2}};
        MultiDimensionalIndexIterator mdiIterEnd{{3, 2}, true};

        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ(mdiIter.limits().size(), 2);
        ASSERT_EQ(mdiIter.limits()[0], 3);
        ASSERT_EQ(mdiIter.limits()[1], 2);

        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ((*mdiIter)[0], 0);
        EXPECT_EQ(mdiIter[0], 0);
        EXPECT_EQ((*mdiIter)[1], 0);
        EXPECT_EQ(mdiIter[1], 0);
        EXPECT_EQ(mdiIter.global(), 0);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ((*mdiIter)[0], 0);
        EXPECT_EQ(mdiIter[0], 0);
        EXPECT_EQ((*mdiIter)[1], 1);
        EXPECT_EQ(mdiIter[1], 1);
        EXPECT_EQ(mdiIter.global(), 1);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ((*mdiIter)[0], 1);
        EXPECT_EQ(mdiIter[0], 1);
        EXPECT_EQ((*mdiIter)[1], 0);
        EXPECT_EQ(mdiIter[1], 0);
        EXPECT_EQ(mdiIter.global(), 2);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ((*mdiIter)[0], 1);
        EXPECT_EQ(mdiIter[0], 1);
        EXPECT_EQ((*mdiIter)[1], 1);
        EXPECT_EQ(mdiIter[1], 1);
        EXPECT_EQ(mdiIter.global(), 3);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ((*mdiIter)[0], 2);
        EXPECT_EQ(mdiIter[0], 2);
        EXPECT_EQ((*mdiIter)[1], 0);
        EXPECT_EQ(mdiIter[1], 0);
        EXPECT_EQ(mdiIter.global(), 4);

        ++mdiIter;
        ASSERT_FALSE(mdiIter == mdiIterEnd);
        ASSERT_EQ((*mdiIter).size(), 2);
        EXPECT_EQ((*mdiIter)[0], 2);
        EXPECT_EQ(mdiIter[0], 2);
        EXPECT_EQ((*mdiIter)[1], 1);
        EXPECT_EQ(mdiIter[1], 1);
        EXPECT_EQ(mdiIter.global(), 5);

        ++mdiIter;
        EXPECT_TRUE(mdiIter == mdiIterEnd);
    }

}