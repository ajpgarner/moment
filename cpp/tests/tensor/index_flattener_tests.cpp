/**
 * index_flattener_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "tensor/index_flattener.h"

namespace Moment::Tests {


    TEST(Tensor_IndexFlattener, Empty) {
        IndexFlattener empty{std::vector<size_t>{}, std::vector<std::vector<size_t>>{}};

        EXPECT_EQ(empty.object.Dimensions.size(), 0);
        EXPECT_EQ(empty.object.DimensionCount, 0);

        EXPECT_EQ(empty.begin(), empty.end());
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
    }

    TEST(Tensor_IndexFlattener, Flatten_Linear) {
        IndexFlattener linear{std::vector<size_t>{5},
                              std::vector<std::vector<size_t>>{std::vector<size_t>{0, 1, 2, 3, 4}}};
        ASSERT_EQ(linear.object.Dimensions.size(), 1);
        ASSERT_EQ(linear.object.DimensionCount, 1);
        ASSERT_EQ(linear.object.Strides.size(), 1);
        EXPECT_EQ(linear.object.Strides[0], 1);
        EXPECT_FALSE(linear.empty());
        EXPECT_EQ(linear.size(), 5);

        auto iter = linear.begin();
        const auto iter_end = linear.end();

        for (size_t n = 0; n < 5; ++n) {
            auto index = iter.index();
            ASSERT_EQ(index.size(), 1) << "n = " << n;
            EXPECT_EQ(index[0], n) << "n = " << n;

            ASSERT_NE(iter, iter_end) << "n = " << n;
            EXPECT_EQ(*iter, n) << "n = " << n;
            ++iter;
        }
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_IndexFlattener, Flatten_Square) {
        IndexFlattener square{std::vector<size_t>{2, 2},
                              std::vector<std::vector<size_t>>{std::vector<size_t>{0, 1},
                                                               std::vector<size_t>{0, 1}}};
        ASSERT_EQ(square.object.Dimensions.size(), 2);
        ASSERT_EQ(square.object.DimensionCount, 2);
        ASSERT_EQ(square.object.Strides.size(), 2);
        EXPECT_EQ(square.object.Strides[0], 1);
        EXPECT_EQ(square.object.Strides[1], 2);

        EXPECT_FALSE(square.empty());
        EXPECT_EQ(square.size(), 4);

        auto iter = square.begin();
        const auto iter_end = square.end();


        for (size_t n = 0; n < 4; ++n) {
            ASSERT_NE(iter, iter_end) << "n = " << n;
            EXPECT_EQ(*iter, n) << "n = " << n;
            ++iter;
        }
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_IndexFlattener, Flatten_Subrectangle) {
        IndexFlattener square{std::vector<size_t>{20, 10},
                              std::vector<std::vector<size_t>>{std::vector<size_t>{10, 11, 12},
                                                               std::vector<size_t>{5, 6}}};
        ASSERT_EQ(square.object.Dimensions.size(), 2);
        ASSERT_EQ(square.object.DimensionCount, 2);
        ASSERT_EQ(square.object.Strides.size(), 2);
        EXPECT_EQ(square.object.Strides[0], 1);
        EXPECT_EQ(square.object.Strides[1], 20);

        EXPECT_FALSE(square.empty());
        EXPECT_EQ(square.size(), 6);

        auto iter = square.begin();
        const auto iter_end = square.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 110); // (10, 5)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 111); // (11, 5)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 112); // (12, 5)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 130); // (10, 6)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 131); // (11, 6)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 132); // (12, 6)
        ++iter;

        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_IndexFlattener, Flatten_DanglingIndices) {
        IndexFlattener tensor{std::vector<size_t>{2, 20, 5},
                              std::vector<std::vector<size_t>>{std::vector<size_t>{1},
                                                               std::vector<size_t>{10, 11, 12, 40}}};

        ASSERT_EQ(tensor.object.Dimensions.size(), 3);
        ASSERT_EQ(tensor.object.DimensionCount, 3);
        ASSERT_EQ(tensor.object.Strides.size(), 3);
        EXPECT_EQ(tensor.object.Strides[0], 1);
        EXPECT_EQ(tensor.object.Strides[1], 2);
        EXPECT_EQ(tensor.object.Strides[2], 40);
        EXPECT_EQ(tensor.object.ElementCount, 200);

        EXPECT_FALSE(tensor.empty());
        EXPECT_EQ(tensor.size(), 4);

        auto iter = tensor.begin();
        const auto iter_end = tensor.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 21); // (1, 10, 0)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 23); // (1, 11, 0)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 25); // (1, 12, 0)
        ++iter;

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, 81); // (1, 40) -> (1, 0, 2)
        ++iter;
    }
}