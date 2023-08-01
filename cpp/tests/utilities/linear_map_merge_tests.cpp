/**
 * linear_map_merge_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/linear_map_merge.h"

#include <map>
#include <string>

namespace Moment::Tests {

    void assert_ABC(const std::map<size_t, std::string>& map) {
        ASSERT_EQ(map.size(), 3);
        auto iter = map.begin();
        EXPECT_EQ(iter->first, 0);
        EXPECT_EQ(iter->second, "A");
        ++iter;
        EXPECT_EQ(iter->first, 10);
        EXPECT_EQ(iter->second, "B");
        ++iter;
        EXPECT_EQ(iter->first, 20);
        EXPECT_EQ(iter->second, "C");
        ++iter;
        EXPECT_EQ(iter, map.end());
    }

    void assert_ABCD(const std::map<size_t, std::string>& map) {
        ASSERT_EQ(map.size(), 4);
        auto iter = map.begin();
        EXPECT_EQ(iter->first, 0);
        EXPECT_EQ(iter->second, "A");
        ++iter;
        EXPECT_EQ(iter->first, 10);
        EXPECT_EQ(iter->second, "B");
        ++iter;
        EXPECT_EQ(iter->first, 20);
        EXPECT_EQ(iter->second, "C");
        ++iter;
        EXPECT_EQ(iter->first, 30);
        EXPECT_EQ(iter->second, "D");
        ++iter;
        EXPECT_EQ(iter, map.end());
    }

    TEST(Utilities_LinearMapMerge, EmptyMaps) {
        std::map<size_t, std::string> mapA;
        std::map<size_t, std::string> mapB;

        linear_map_merge(mapA, std::move(mapB));
        EXPECT_TRUE(mapA.empty());
    }

    TEST(Utilities_LinearMapMerge, EmptyLHS) {
        std::map<size_t, std::string> mapA;
        std::map<size_t, std::string> mapB;
        mapB.insert({0, "Hello"});
        mapB.insert({10, "World"});

        linear_map_merge(mapA, std::move(mapB));
        ASSERT_EQ(mapA.size(), 2);
        auto iter = mapA.begin();
        EXPECT_EQ(iter->first, 0);
        EXPECT_EQ(iter->second, "Hello");
        ++iter;
        EXPECT_EQ(iter->first, 10);
        EXPECT_EQ(iter->second, "World");
        ++iter;
        EXPECT_EQ(iter, mapA.end());

    }

    TEST(Utilities_LinearMapMerge, EmptyRHS) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "Hello"});
        mapA.insert({10, "World"});
        std::map<size_t, std::string> mapB;

        linear_map_merge(mapA, std::move(mapB));
        ASSERT_EQ(mapA.size(), 2);
        auto iter = mapA.begin();
        EXPECT_EQ(iter->first, 0);
        EXPECT_EQ(iter->second, "Hello");
        ++iter;
        EXPECT_EQ(iter->first, 10);
        EXPECT_EQ(iter->second, "World");
        ++iter;
        EXPECT_EQ(iter, mapA.end());
    }



    TEST(Utilities_LinearMapMerge, AABB) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "A"});
        mapA.insert({10, "B"});
        std::map<size_t, std::string> mapB;
        mapB.insert({20, "C"});
        mapB.insert({30, "D"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABCD(mapA);
    }

    TEST(Utilities_LinearMapMerge, BBAA) {
        std::map<size_t, std::string> mapA;
        mapA.insert({20, "C"});
        mapA.insert({30, "D"});

        std::map<size_t, std::string> mapB;
        mapB.insert({0, "A"});
        mapB.insert({10, "B"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABCD(mapA);
    }

    TEST(Utilities_LinearMapMerge, ABAB) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "A"});
        mapA.insert({20, "C"});

        std::map<size_t, std::string> mapB;
        mapB.insert({10, "B"});
        mapB.insert({30, "D"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABCD(mapA);
    }

    TEST(Utilities_LinearMapMerge, ABBA) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "A"});
        mapA.insert({30, "D"});

        std::map<size_t, std::string> mapB;
        mapB.insert({10, "B"});
        mapB.insert({20, "C"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABCD(mapA);

    }

    TEST(Utilities_LinearMapMerge, BABA) {
        std::map<size_t, std::string> mapA;
        mapA.insert({10, "B"});
        mapA.insert({30, "D"});

        std::map<size_t, std::string> mapB;
        mapB.insert({0, "A"});
        mapB.insert({20, "C"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABCD(mapA);

    }

    TEST(Utilities_LinearMapMerge, BAAB) {
        std::map<size_t, std::string> mapA;
        mapA.insert({10, "B"});
        mapA.insert({20, "C"});

        std::map<size_t, std::string> mapB;
        mapB.insert({0, "A"});
        mapB.insert({30, "D"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABCD(mapA);

    }
    TEST(Utilities_LinearMapMerge, Overlap_Start) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "A"});
        mapA.insert({10, "B"});

        std::map<size_t, std::string> mapB;
        mapB.insert({0, "A"});
        mapB.insert({20, "C"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABC(mapA);
    }
    TEST(Utilities_LinearMapMerge, Overlap_Mid) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "A"});
        mapA.insert({10, "B"});

        std::map<size_t, std::string> mapB;
        mapB.insert({10, "B"});
        mapB.insert({20, "C"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABC(mapA);
    }

    TEST(Utilities_LinearMapMerge, Overlap_End) {
        std::map<size_t, std::string> mapA;
        mapA.insert({0, "A"});
        mapA.insert({20, "C"});

        std::map<size_t, std::string> mapB;
        mapB.insert({10, "B"});
        mapB.insert({20, "C"});

        linear_map_merge(mapA, std::move(mapB));
        assert_ABC(mapA);
    }


}