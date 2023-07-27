/**
 * auto_storage_tensor_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"
#include "tensor/auto_storage_tensor.h"

namespace Moment::Tests {

    class BoringTensor : public AutoStorageTensor<int, 5> {
    public:
        explicit BoringTensor(std::vector<size_t>&& dims, TensorStorageType tst = TensorStorageType::Automatic)
            : AutoStorageTensor<int, 5>(std::move(dims), tst) {

            if (this->StorageType == TensorStorageType::Explicit) {
                for (int i = 0; i < this->ElementCount; ++i) {
                    this->data.emplace_back(i);
                }
            }
        }

    private:
        [[nodiscard]] int make_element_no_checks(AutoStorageIndexView index) const override {
            return static_cast<int>(this->index_to_offset_no_checks(index));
        }

        [[nodiscard]] std::string get_name(bool capital) const override {
            if (capital) {
                return "Boring tensor";
            } else {
                return "boring tensor";
            }
        }
    };

    TEST(Tensor_AutoStorage, AutoStorageDeduction) {
        BoringTensor tensor31({3, 1});
        ASSERT_EQ(tensor31.StorageType, TensorStorageType::Explicit);

        BoringTensor tensor31_overload({3, 1}, TensorStorageType::Virtual);
        ASSERT_EQ(tensor31_overload.StorageType, TensorStorageType::Virtual);

        BoringTensor tensor32({3, 2});
        ASSERT_EQ(tensor32.StorageType, TensorStorageType::Virtual);

        BoringTensor tensor32_overload({3, 2}, TensorStorageType::Explicit);
        ASSERT_EQ(tensor32_overload.StorageType, TensorStorageType::Explicit);
    }


    TEST(Tensor_AutoStorage, VirtualMode) {
        BoringTensor auto_deduce({4, 3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        EXPECT_THROW([[maybe_unused]] const auto& data = auto_deduce.Data(), errors::bad_tensor);

        EXPECT_EQ(auto_deduce(AutoStorageIndex{0, 0, 0}), 0);
        EXPECT_EQ(auto_deduce(AutoStorageIndex{2, 2, 2}), 34); // 2 + 2*4 + 2*4*3 = 34

    }

    TEST(Tensor_AutoStorage, Iterator_Explicit_Full) {
        BoringTensor auto_deduce({2, 2});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::Iterator iter(auto_deduce, AutoStorageIndex{0, 0}, AutoStorageIndex{2, 2});
        BoringTensor::Iterator iter_end(auto_deduce);

        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 0);
        EXPECT_EQ(iter.offset(), 0);
        EXPECT_EQ(*iter, 0);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 1);
        EXPECT_EQ(iter.offset(), 1);
        EXPECT_EQ(*iter, 1);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 2);
        EXPECT_EQ(iter.offset(), 2);
        EXPECT_EQ(*iter, 2);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 3);
        EXPECT_EQ(iter.offset(), 3);
        EXPECT_EQ(*iter, 3);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_AutoStorage, Iterator_Explicit_Row) {
        BoringTensor auto_deduce({2, 2});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::Iterator iter(auto_deduce, AutoStorageIndex{1, 0}, AutoStorageIndex{2, 2});
        BoringTensor::Iterator iter_end(auto_deduce);

        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 0);
        EXPECT_EQ(iter.offset(), 1);
        EXPECT_EQ(*iter, 1);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 1);
        EXPECT_EQ(iter.offset(), 3);
        EXPECT_EQ(*iter, 3);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_AutoStorage, Iterator_Explicit_Col) {
        BoringTensor auto_deduce({2, 2});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::Iterator iter(auto_deduce, AutoStorageIndex{0, 1}, AutoStorageIndex{2, 2});
        BoringTensor::Iterator iter_end(auto_deduce);

        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 0);
        EXPECT_EQ(iter.offset(), 2);
        EXPECT_EQ(*iter, 2);

        ++iter;
        EXPECT_NE(iter, iter_end);
        EXPECT_EQ(iter.block_offset(), 1);
        EXPECT_EQ(iter.offset(), 3);
        EXPECT_EQ(*iter, 3);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_AutoStorage, Iterator_Virtual_Full) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::Iterator iter(auto_deduce, AutoStorageIndex{0, 0}, AutoStorageIndex{3, 3});
        BoringTensor::Iterator iter_end(auto_deduce);
        for (size_t iteration = 0; iteration < 9; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_offset(), iteration);
            EXPECT_EQ(iter.offset(), iteration);
            EXPECT_EQ(*iter, iteration);
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_AutoStorage, Iterator_Virtual_Row) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::Iterator iter(auto_deduce, AutoStorageIndex{1, 0}, AutoStorageIndex{2, 3});
        BoringTensor::Iterator iter_end(auto_deduce);
        for (size_t iteration = 0; iteration < 3; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_offset(), iteration);
            EXPECT_EQ(iter.offset(), 1+(3*iteration));
            EXPECT_EQ(*iter, 1+(3*iteration));
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_AutoStorage, Iterator_Virtual_Col) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::Iterator iter(auto_deduce, AutoStorageIndex{0, 1}, AutoStorageIndex{3, 2});
        BoringTensor::Iterator iter_end(auto_deduce);
        for (size_t iteration = 0; iteration < 3; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_offset(), iteration);
            EXPECT_EQ(iter.offset(), 3+iteration);
            EXPECT_EQ(*iter, 3+iteration);
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_AutoStorage, Range_Col) {
        BoringTensor auto_deduce({3, 3});
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        auto range = auto_deduce.Splice(AutoStorageIndex{0, 1}, AutoStorageIndex{3, 2});

        auto iter = range.begin();
        const auto iter_end = range.end();
        for (size_t iteration = 0; iteration < 3; ++iteration) {
            ASSERT_NE(iter, iter_end) << "Iteration = " << iteration;
            EXPECT_EQ(iter.block_offset(), iteration);
            EXPECT_EQ(iter.offset(), 3+iteration);
            EXPECT_EQ(*iter, 3+iteration);
            ++iter;
        }

        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_AutoStorage, Range_SliceDimension) {
        BoringTensor auto_deduce({5, 5});
        auto range = auto_deduce.Splice(AutoStorageIndex{2, 3}, AutoStorageIndex{5, 5}); // 3 x 2 slice

        auto range_dims = range.Dimensions();
        EXPECT_EQ(range_dims.size(), 2);
        EXPECT_EQ(range_dims[0], 3);
        EXPECT_EQ(range_dims[1], 2);
    }

    TEST(Tensor_AutoStorage, Range_SliceSize) {
        BoringTensor auto_deduce({5, 5});
        auto range = auto_deduce.Splice(AutoStorageIndex{2, 3}, AutoStorageIndex{5, 5}); // 3 x 2 slice

        EXPECT_EQ(range.size(), 6);
    }


    TEST(Tensor_AutoStorage, View_Explicit) {
        BoringTensor auto_deduce({3, 3}, TensorStorageType::Explicit);
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Explicit);

        BoringTensor::ElementView view{auto_deduce, AutoStorageIndex{1, 1}};

        EXPECT_EQ(view, 4);

    }

    TEST(Tensor_AutoStorage, View_Virtual) {
        BoringTensor auto_deduce({3, 3}, TensorStorageType::Virtual);
        ASSERT_EQ(auto_deduce.StorageType, TensorStorageType::Virtual);

        BoringTensor::ElementView view{auto_deduce, AutoStorageIndex{1, 1}};
        EXPECT_EQ(view, 4);
    }

    TEST(Tensor_AutoStorage, FullIterator_Explicit) {
        BoringTensor tensor({2, 2}, TensorStorageType::Explicit);
        ASSERT_EQ(tensor.StorageType, TensorStorageType::Explicit);

        auto full_iter = tensor.begin();
        const auto full_iter_end = tensor.end();
        ASSERT_TRUE(full_iter.explicit_mode());

        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 0);
        EXPECT_EQ(full_iter.offset(), 0);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{0, 0}));

        ++full_iter;
        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 1);
        EXPECT_EQ(full_iter.offset(), 1);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{1, 0}));

        ++full_iter;
        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 2);
        EXPECT_EQ(full_iter.offset(), 2);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{0, 1}));

        ++full_iter;
        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 3);
        EXPECT_EQ(full_iter.offset(), 3);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{1, 1}));

        ++full_iter;
        EXPECT_EQ(full_iter, full_iter_end);
    }

    TEST(Tensor_AutoStorage, FullIterator_Virtual) {
        BoringTensor tensor({2, 2}, TensorStorageType::Virtual);
        ASSERT_EQ(tensor.StorageType, TensorStorageType::Virtual);

        auto full_iter = tensor.begin();
        const auto full_iter_end = tensor.end();
        ASSERT_FALSE(full_iter.explicit_mode());

        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 0);
        EXPECT_EQ(full_iter.offset(), 0);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{0, 0}));

        ++full_iter;
        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 1);
        EXPECT_EQ(full_iter.offset(), 1);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{1, 0}));

        ++full_iter;
        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 2);
        EXPECT_EQ(full_iter.offset(), 2);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{0, 1}));

        ++full_iter;
        ASSERT_NE(full_iter, full_iter_end);
        EXPECT_EQ(*full_iter, 3);
        EXPECT_EQ(full_iter.offset(), 3);
        EXPECT_EQ(full_iter.index(), (AutoStorageIndex{1, 1}));

        ++full_iter;
        EXPECT_EQ(full_iter, full_iter_end);
    }


}
