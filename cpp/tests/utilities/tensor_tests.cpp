/**
 * tensor_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"
#include "utilities/tensor.h"

namespace Moment::Tests {

    class BoringTensor : public AutoStorageTensor<int, 5> {
    public:
        BoringTensor(std::vector<size_t>&& dims, TensorStorageType tst = TensorStorageType::Automatic)
            : AutoStorageTensor<int, 5>(std::move(dims), tst) { }

    private:
        [[nodiscard]] int make_element_no_checks(Tensor::IndexView index) const override {
            return static_cast<int>(this->index_to_offset_no_checks(index));
        }

        [[nodiscard]] std::string get_name() const override {
            return "Boring tensor";
        }
    };

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
        EXPECT_THROW([[maybe_unused]] auto o = tensor.index_to_offset(Tensor::Index{3}), errors::bad_tensor_index);
        EXPECT_THROW([[maybe_unused]] auto o = tensor.index_to_offset(Tensor::Index{0, 0}), errors::bad_tensor_index);
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

        EXPECT_THROW([[maybe_unused]] auto o = tensor.index_to_offset(Tensor::Index{3, 0}),
                     errors::bad_tensor_index);
        EXPECT_THROW([[maybe_unused]] auto o = tensor.index_to_offset(Tensor::Index{0, 0, 0}),
                     errors::bad_tensor_index);
    }



    TEST(Utilities_Tensor, OffsetToIndex) {
        Tensor tensor({3, 2});
        auto elem00 = tensor.offset_to_index(0);
        ASSERT_EQ(elem00.size(), 2);
        EXPECT_EQ(elem00[0], 0);
        EXPECT_EQ(elem00[1], 0);
        EXPECT_EQ(tensor.index_to_offset(elem00), 0);

        auto elem10 = tensor.offset_to_index(1);
        ASSERT_EQ(elem10.size(), 2);
        EXPECT_EQ(elem10[0], 1);
        EXPECT_EQ(elem10[1], 0);
        EXPECT_EQ(tensor.index_to_offset(elem10), 1);

        auto elem20 = tensor.offset_to_index(2);
        ASSERT_EQ(elem20.size(), 2);
        EXPECT_EQ(elem20[0], 2);
        EXPECT_EQ(elem20[1], 0);
        EXPECT_EQ(tensor.index_to_offset(elem20), 2);

        auto elem01 = tensor.offset_to_index(3);
        ASSERT_EQ(elem01.size(), 2);
        EXPECT_EQ(elem01[0], 0);
        EXPECT_EQ(elem01[1], 1);
        EXPECT_EQ(tensor.index_to_offset(elem01), 3);

        auto elem11 = tensor.offset_to_index(4);
        ASSERT_EQ(elem11.size(), 2);
        EXPECT_EQ(elem11[0], 1);
        EXPECT_EQ(elem11[1], 1);
        EXPECT_EQ(tensor.index_to_offset(elem11), 4);

        auto elem21 = tensor.offset_to_index(5);
        ASSERT_EQ(elem21.size(), 2);
        EXPECT_EQ(elem21[0], 2);
        EXPECT_EQ(elem21[1], 1);
        EXPECT_EQ(tensor.index_to_offset(elem21), 5);

        EXPECT_THROW([[maybe_unused]] auto o = tensor.offset_to_index(6), errors::bad_tensor_index);

    }

    TEST(Utilities_Tensor, AutoStorageDeduction) {
        BoringTensor tensor31({3, 1});
        ASSERT_EQ(tensor31.StorageType, TensorStorageType::Explicit);

        BoringTensor tensor31_overload({3, 1}, TensorStorageType::Virtual);
        ASSERT_EQ(tensor31_overload.StorageType, TensorStorageType::Virtual);

        BoringTensor tensor32({3, 2});
        ASSERT_EQ(tensor32.StorageType, TensorStorageType::Virtual);

        BoringTensor tensor32_overload({3, 2}, TensorStorageType::Explicit);
        ASSERT_EQ(tensor32_overload.StorageType, TensorStorageType::Explicit);
    }


    TEST(Utilities_Tensor, VirtualMode) {
        BoringTensor auto_deduce({4, 3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        EXPECT_THROW([[maybe_unused]] const auto& data = auto_deduce.Data(), errors::bad_tensor);

        EXPECT_EQ(auto_deduce(Tensor::Index{0, 0, 0}), 0);
        EXPECT_EQ(auto_deduce(Tensor::Index{2, 2, 2}), 34); // 2 + 2*4 + 2*4*3 = 34

    }

}
