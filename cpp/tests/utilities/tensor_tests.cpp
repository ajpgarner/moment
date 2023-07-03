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
            : AutoStorageTensor<int, 5>(std::move(dims), tst) {

            if (this->StorageType == TensorStorageType::Explicit) {
                for (int i = 0; i < this->ElementCount; ++i) {
                    this->data.emplace_back(i);
                }
            }
        }

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

    TEST(Utilities_Tensor, Iterator_Explicit_Full) {
        BoringTensor auto_deduce({2, 2});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::Iterator iter(auto_deduce, Tensor::Index{0, 0}, Tensor::Index{2, 2});
        BoringTensor::Iterator iter_end(auto_deduce);

        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 0);
        EXPECT_EQ(iter.offset(), 0);
        EXPECT_EQ(*iter, 0);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 1);
        EXPECT_EQ(iter.offset(), 1);
        EXPECT_EQ(*iter, 1);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 2);
        EXPECT_EQ(iter.offset(), 2);
        EXPECT_EQ(*iter, 2);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 3);
        EXPECT_EQ(iter.offset(), 3);
        EXPECT_EQ(*iter, 3);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Utilities_Tensor, Iterator_Explicit_Row) {
        BoringTensor auto_deduce({2, 2});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::Iterator iter(auto_deduce, Tensor::Index{1, 0}, Tensor::Index{2, 2});
        BoringTensor::Iterator iter_end(auto_deduce);

        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 0);
        EXPECT_EQ(iter.offset(), 1);
        EXPECT_EQ(*iter, 1);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 1);
        EXPECT_EQ(iter.offset(), 3);
        EXPECT_EQ(*iter, 3);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Utilities_Tensor, Iterator_Explicit_Col) {
        BoringTensor auto_deduce({2, 2});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::Iterator iter(auto_deduce, Tensor::Index{0, 1}, Tensor::Index{2, 2});
        BoringTensor::Iterator iter_end(auto_deduce);

        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 0);
        EXPECT_EQ(iter.offset(), 2);
        EXPECT_EQ(*iter, 2);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_index(), 1);
        EXPECT_EQ(iter.offset(), 3);
        EXPECT_EQ(*iter, 3);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Utilities_Tensor, Iterator_Virtual_Full) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::Iterator iter(auto_deduce, Tensor::Index{0, 0}, Tensor::Index{3, 3});
        BoringTensor::Iterator iter_end(auto_deduce);
        for (size_t iteration = 0; iteration < 9; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_index(), iteration);
            EXPECT_EQ(iter.offset(), iteration);
            EXPECT_EQ(*iter, iteration);
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }

    TEST(Utilities_Tensor, Iterator_Virtual_Row) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::Iterator iter(auto_deduce, Tensor::Index{1, 0}, Tensor::Index{2, 3});
        BoringTensor::Iterator iter_end(auto_deduce);
        for (size_t iteration = 0; iteration < 3; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_index(), iteration);
            EXPECT_EQ(iter.offset(), 1+(3*iteration));
            EXPECT_EQ(*iter, 1+(3*iteration));
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }

    TEST(Utilities_Tensor, Iterator_Virtual_Col) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::Iterator iter(auto_deduce, Tensor::Index{0, 1}, Tensor::Index{3, 2});
        BoringTensor::Iterator iter_end(auto_deduce);
        for (size_t iteration = 0; iteration < 3; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_index(), iteration);
            EXPECT_EQ(iter.offset(), 3+iteration);
            EXPECT_EQ(*iter, 3+iteration);
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }


}
