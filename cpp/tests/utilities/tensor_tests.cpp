/**
 * tensor_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"
#include "utilities/tensor.h"

namespace Moment::Tests {

    TEST(Utilities_Tensor, Empty) {
        Tensor empty({});
        EXPECT_TRUE(empty.Strides.empty());
        EXPECT_TRUE(empty.Dimensions.empty());
        EXPECT_EQ(empty.DimensionCount, 0);
        EXPECT_EQ(empty.ElementCount, 0);
    }

    TEST(Utilities_Tensor, OneDimension) {
        Tensor tensor({3});
        ASSERT_EQ(tensor.DimensionCount, 1);
        EXPECT_EQ(tensor.ElementCount, 3);
        ASSERT_EQ(tensor.Strides.size(), 1);
        ASSERT_EQ(tensor.Dimensions.size(), 1);
        EXPECT_EQ(tensor.Dimensions[0], 3);
        EXPECT_EQ(tensor.Strides[0], 1);

        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{0}), 0);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{1}), 1);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{2}), 2);
        EXPECT_THROW(tensor.index_to_offset(Tensor::Index{3}), errors::bad_tensor_index);
        EXPECT_THROW(tensor.index_to_offset(Tensor::Index{0, 0}), errors::bad_tensor_index);
    }

    TEST(Utilities_Tensor, TwoDimensions) {
        Tensor tensor({3, 2});
        ASSERT_EQ(tensor.DimensionCount, 2);
        EXPECT_EQ(tensor.ElementCount, 6);
        ASSERT_EQ(tensor.Strides.size(), 2);
        ASSERT_EQ(tensor.Dimensions.size(), 2);
        EXPECT_EQ(tensor.Dimensions[0], 3);
        EXPECT_EQ(tensor.Dimensions[1], 2);
        EXPECT_EQ(tensor.Strides[0], 1);
        EXPECT_EQ(tensor.Strides[1], 3);

        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{0, 0}), 0);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{1, 0}), 1);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{2, 0}), 2);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{0, 1}), 3);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{1, 1}), 4);
        EXPECT_EQ(tensor.index_to_offset(Tensor::Index{2, 1}), 5);

        EXPECT_THROW(tensor.index_to_offset(Tensor::Index{3, 0}), errors::bad_tensor_index);
        EXPECT_THROW(tensor.index_to_offset(Tensor::Index{0, 0, 0}), errors::bad_tensor_index);
    }


}
