/**
 * variable_chunk_range_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"
#include "utilities/variable_chunk_range.h"

#include <string>
#include <vector>

namespace Moment::Tests {

    TEST(Utilities_VariableChunkRange, Empty) {
        std::vector<std::string> data{};
        std::vector<size_t> indices{};

        VariableChunkRange<std::string, size_t> vcr{data, indices};

        EXPECT_EQ(vcr.begin(), vcr.end());
        EXPECT_TRUE(vcr.begin() == vcr.end());
        EXPECT_FALSE(vcr.begin() != vcr.end());

    }

    TEST(Utilities_VariableChunkRange, SameSize) {

        std::vector<std::string> data{"Apple", "Orange", "Cherry", "Pear"};
        std::vector<size_t> indices{0, 1, 2, 3};

        VariableChunkRange<std::string> vcr{data, indices};

        auto iter = vcr.begin();
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[0]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[1]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[2]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[3]);

        ++iter;
        EXPECT_EQ(iter, vcr.end());
    }

    TEST(Utilities_VariableChunkRange, VariableSizes) {

        std::vector<std::string> data{"Apple", "Orange", "Cherry", "Pear"};
        std::vector<size_t> indices{0, 1, 3};

        VariableChunkRange<std::string> vcr{data, indices};

        auto iter = vcr.begin();
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[0]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 2);
        EXPECT_EQ((*iter).size(), 2);
        EXPECT_EQ((*iter).data(), &data[1]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[3]);

        ++iter;
        EXPECT_EQ(iter, vcr.end());
    }

    TEST(Utilities_VariableChunkRange, VariableSizes2) {

        std::vector<std::string> data{"Apple", "Orange", "Cherry", "Pear", "Carrot"};
        std::vector<size_t> indices{0, 1, 3};

        VariableChunkRange<std::string> vcr{data, indices};

        auto iter = vcr.begin();
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[0]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 2);
        EXPECT_EQ((*iter).size(), 2);
        EXPECT_EQ((*iter).data(), &data[1]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 2);
        EXPECT_EQ((*iter).size(), 2);
        EXPECT_EQ((*iter).data(), &data[3]);

        ++iter;
        EXPECT_EQ(iter, vcr.end());
    }

    TEST(Utilities_VariableChunkRange, NullIndex) {

        std::vector<std::string> data{"Apple", "Orange", "Cherry", "Pear"};
        std::vector<size_t> indices{0, 1, 2, 2, 3};

        VariableChunkRange<std::string> vcr{data, indices};

        auto iter = vcr.begin();
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[0]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[1]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 0);
        EXPECT_EQ((*iter).size(), 0);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[2]);

        ++iter;
        ASSERT_NE(iter, vcr.end());
        EXPECT_EQ(iter.chunk_size(), 1);
        EXPECT_EQ((*iter).size(), 1);
        EXPECT_EQ((*iter).data(), &data[3]);

        ++iter;
        EXPECT_EQ(iter, vcr.end());
    }
}