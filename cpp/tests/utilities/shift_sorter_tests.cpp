/**
 * recursive_storage_tests.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/shift_sorter.h"


namespace Moment::Tests {

    TEST(Utilities_ShiftSorter, Empty) {
        ShiftSorter<int> ss;
        EXPECT_EQ(ss({}), 0);
    }

    TEST(Utilities_ShiftSorter, Length1) {
        ShiftSorter<int> ss;
        EXPECT_EQ(ss({0}), 0);
        EXPECT_EQ(ss({1}), 0);
        EXPECT_EQ(ss({10}), 0);
    }

    TEST(Utilities_ShiftSorter, Length2) {
        ShiftSorter<int> ss;

        EXPECT_EQ(ss({0, 3}), 0);
        EXPECT_EQ(ss({3, 0}), 1);
        EXPECT_EQ(ss({0, 0}), 0);
        EXPECT_EQ(ss({3, 3}), 0);
    }

    TEST(Utilities_ShiftSorter, Length3) {
        ShiftSorter<int> ss;
        EXPECT_EQ(ss({1, 1, 1}), 0);
        EXPECT_EQ(ss({1, 1, 2}), 0);
        EXPECT_EQ(ss({1, 2, 1}), 2);
        EXPECT_EQ(ss({2, 1, 1}), 1);
        EXPECT_EQ(ss({1, 2, 2}), 0);
        EXPECT_EQ(ss({2, 1, 2}), 1);
        EXPECT_EQ(ss({2, 2, 1}), 2);
        EXPECT_EQ(ss({2, 2, 2}), 0);
    }

    TEST(Utilities_ShiftSorter, Length4) {
        ShiftSorter<int> ss;
        EXPECT_EQ(ss({1, 1, 1, 1}), 0);
        EXPECT_EQ(ss({1, 2, 1, 2}), 0);
        EXPECT_EQ(ss({2, 1, 2, 1}), 1);
        EXPECT_EQ(ss({2, 1, 1, 2}), 1);
        EXPECT_EQ(ss({2, 2, 1, 1}), 2);
        EXPECT_EQ(ss({1, 2, 2, 1}), 3);
    }

}