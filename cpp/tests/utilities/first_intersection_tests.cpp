/**
 * first_intersection_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/first_intersection_index.h"

#include <string>
#include <vector>

namespace Moment::Tests {
    TEST(Utilities_FirstIntersection, Empty_Nothing) {
        const std::set<int> test{};
        const std::set<int> reference{};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        EXPECT_FALSE(result.has_value());

    }

    TEST(Utilities_FirstIntersection, Empty_NoMatch) {
        const std::set<int> test{1, 2, 3};
        const std::set<int> reference{};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        EXPECT_FALSE(result.has_value());

    }

    TEST(Utilities_FirstIntersection, Empty_NoTest) {
        const std::set<int> test{};
        const std::set<int> reference{1, 2, 3};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        EXPECT_FALSE(result.has_value());

    }

    TEST(Utilities_FirstIntersection, Ints_MatchIndex0) {
        const std::set<int> test{1, 2, 3};
        const std::set<int> reference{1, 2, 3};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 0); // index 0: 1
    }

    TEST(Utilities_FirstIntersection, Ints_MatchIndex0_Interior) {
        const std::set<int> test{0, 1, 2, 3};
        const std::set<int> reference{1, 2, 3};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 0); // index 0: 1
    }

    TEST(Utilities_FirstIntersection, Ints_MatchIndex1) {
        const std::set<int> test{2, 3, 4};
        const std::set<int> reference{1, 2, 3};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 1); // index 1: 2
    }

    TEST(Utilities_FirstIntersection, Ints_MatchIndex1_Interior) {
        const std::set<int> test{0, 2, 3};
        const std::set<int> reference{1, 2, 3};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 1); // index 1: 2
    }


    TEST(Utilities_FirstIntersection, Strings) {
        const std::set<std::string> test{"This", "is", "the", "test", "string"}; // NB: will be sorted.
        const std::set<std::string> reference{"cases", "test"};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        ASSERT_TRUE(result.has_value());
        EXPECT_EQ(result.value(), 1); // index 1: "test"
    }

    TEST(Utilities_FirstIntersection, Strings_NoMatch) {
        const std::set<std::string> test{"This", "is", "the", "test", "string"}; // NB: will be sorted.
        const std::set<std::string> reference{"match", "nothing"};

        auto result = first_intersection_index(test.begin(), test.end(), reference.begin(), reference.end());
        EXPECT_FALSE(result.has_value());
    }


}