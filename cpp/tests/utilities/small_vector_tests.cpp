/**
 * small_vector_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/small_vector.h"

#include <vector>

namespace Moment::Tests {

    TEST(Utilities_SmallVector, Construct_Empty) {
        SmallVector<double, 5> empty;
        EXPECT_TRUE(empty.empty());
        EXPECT_EQ(empty.size(), 0);
        EXPECT_EQ(empty.capacity(), 5);
        EXPECT_FALSE(empty.on_heap());
    }

    TEST(Utilities_SmallVector, Construct_InitListSmall) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        EXPECT_EQ(small.size(), 3);
        ASSERT_EQ(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_FALSE(small.on_heap());
    }

    TEST(Utilities_SmallVector, Construct_InitListLarge) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 6);
        EXPECT_GE(small.capacity(), 6);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(small[5], 6.0);
        EXPECT_TRUE(small.on_heap());
    }

    TEST(Utilities_SmallVector, Construct_FromIteratorsSmall) {
        std::vector<double> src{1.0, 2.0, 3.0};
        SmallVector<double, 5> small(src.begin(), src.end());
        EXPECT_FALSE(small.empty());
        EXPECT_EQ(small.size(), 3);
        ASSERT_EQ(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_FALSE(small.on_heap());
    }

    TEST(Utilities_SmallVector, Construct_FromIteratorsLarge) {
        std::vector<double> src{1.0, 2.0, 3.0, 4.0, 5.0, 6.0};
        SmallVector<double, 5> small(src.begin(), src.end());
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 6);
        EXPECT_GE(small.capacity(), 6);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(small[5], 6.0);
        EXPECT_TRUE(small.on_heap());
    }

    TEST(Utilities_SmallVector, CopyConstruct_Stack) {
        const SmallVector<double, 5> small{1.0, 2.0, 3.0};
        ASSERT_FALSE(small.on_heap());

        SmallVector<double, 5> copied_small{small};
        ASSERT_EQ(copied_small.size(), 3);
        ASSERT_GE(copied_small.capacity(), 3);
        EXPECT_FALSE(copied_small.on_heap());

        // Check data copied
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(copied_small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(copied_small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(copied_small[2], 3.0);

        // Check copy is deep (pointers not same!)
        EXPECT_NE(small.get(), copied_small.get());
        copied_small[2] = 4.0;
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(copied_small[2], 4.0);
    }

    TEST(Utilities_SmallVector, CopyConstruct_Heap) {
        const SmallVector<double, 3> small{1.0, 2.0, 3.0, 4.0, 5.0};
        ASSERT_TRUE(small.on_heap());

        SmallVector<double, 3> copied_small{small};
        ASSERT_EQ(copied_small.size(), 5);
        ASSERT_GE(copied_small.capacity(), 5);
        EXPECT_TRUE(copied_small.on_heap());

        // Check data copied
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(copied_small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(copied_small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(copied_small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(copied_small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(copied_small[4], 5.0);

        // Check copy is deep (pointers not same!)
        EXPECT_NE(small.get(), copied_small.get());
        copied_small[2] = 40.0;
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(copied_small[2], 40.0);
    }

    TEST(Utilities_SmallVector, MoveConstruct_Stack) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0};
        ASSERT_FALSE(small.on_heap());

        SmallVector<double, 5> moved_small{std::move(small)};
        ASSERT_EQ(moved_small.size(), 3);
        ASSERT_GE(moved_small.capacity(), 3);
        EXPECT_FALSE(moved_small.on_heap());

        // Check data moved successfully
        EXPECT_EQ(moved_small[0], 1.0);
        EXPECT_EQ(moved_small[1], 2.0);
        EXPECT_EQ(moved_small[2], 3.0);
    }

    TEST(Utilities_SmallVector, MoveConstruct_Heap) {
        SmallVector<double, 3> small{1.0, 2.0, 3.0, 4.0, 5.0};
        ASSERT_TRUE(small.on_heap());

        SmallVector<double, 3> moved_small{std::move(small)};
        ASSERT_EQ(moved_small.size(), 5);
        EXPECT_TRUE(moved_small.on_heap());

        // Check data moved
        EXPECT_EQ(moved_small[0], 1.0);
        EXPECT_EQ(moved_small[1], 2.0);
        EXPECT_EQ(moved_small[2], 3.0);
        EXPECT_EQ(moved_small[3], 4.0);
        EXPECT_EQ(moved_small[4], 5.0);
    }

    TEST(Utilities_SmallVector, PushBack) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 3);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_FALSE(small.on_heap());

        small.push_back(4.0);
        ASSERT_EQ(small.size(), 4);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_FALSE(small.on_heap());

        small.push_back(5.0);
        ASSERT_EQ(small.size(), 5);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_FALSE(small.on_heap());

        small.push_back(6.0);
        ASSERT_EQ(small.size(), 6);
        EXPECT_EQ(small[5], 6.0);
        EXPECT_GE(small.capacity(), 6);
        EXPECT_TRUE(small.on_heap());

        // Re-test earlier values
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);
    }

    TEST(Utilities_SmallVector, PushBack_HeapToHeap) {
        SmallVector<double, 2> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 3);
        auto old_cap = small.capacity();
        EXPECT_GE(old_cap, 3);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        ASSERT_TRUE(small.on_heap());

        ASSERT_LT(old_cap, 10000) << "Unexpectedly large allocation!";
        for (size_t val = 3; val < old_cap+1; ++val) {
            small.push_back(static_cast<double>(val)+1.0);
        }
        ASSERT_EQ(small.size(), old_cap+1);
        EXPECT_GT(small.capacity(), old_cap);

        for (size_t val = 3; val < old_cap+1; ++val) {
            EXPECT_EQ(small[val], static_cast<double>(val)+1.0);
        }
    }

    TEST(Utilities_SmallVector, EmplaceBack_Trivial) {
        SmallVector<double, 5> small{1.0, 2.0, 3.0};
        EXPECT_FALSE(small.empty());
        ASSERT_EQ(small.size(), 3);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_FALSE(small.on_heap());

        small.emplace_back(4.0);
        ASSERT_EQ(small.size(), 4);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_FALSE(small.on_heap());

        small.emplace_back(5.0);
        ASSERT_EQ(small.size(), 5);
        EXPECT_EQ(small[4], 5.0);
        EXPECT_EQ(small.capacity(), 5);
        EXPECT_FALSE(small.on_heap());

        small.emplace_back(6.0);
        ASSERT_EQ(small.size(), 6);
        EXPECT_EQ(small[5], 6.0);
        EXPECT_GE(small.capacity(), 6);
        EXPECT_TRUE(small.on_heap());

        // Re-test earlier values
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);


    }

    TEST(Utilities_SmallVector, EmplaceBack_Complex) {
        SmallVector<std::pair<double, std::string>, 1> trickySmall;
        EXPECT_TRUE(trickySmall.empty());

        trickySmall.emplace_back(13.0, "Hello world");
        ASSERT_EQ(trickySmall.size(), 1);
        EXPECT_EQ(trickySmall[0].first, 13.0);
        EXPECT_EQ(trickySmall[0].second, "Hello world");
        EXPECT_FALSE(trickySmall.on_heap());

        trickySmall.emplace_back(20.0, "Cheesecake");
        ASSERT_EQ(trickySmall.size(), 2);
        EXPECT_GE(trickySmall.capacity(), 2);
        EXPECT_TRUE(trickySmall.on_heap());

        EXPECT_EQ(trickySmall[0].first, 13.0);
        EXPECT_EQ(trickySmall[0].second, "Hello world");
        EXPECT_EQ(trickySmall[1].first, 20.0);
        EXPECT_EQ(trickySmall[1].second, "Cheesecake");


    }

    TEST(Utilities_SmallVector, Iterator) {
        SmallVector<double, 4> small{1.0, 2.0, 3.0};

        auto iter = small.begin();
        ASSERT_NE(iter, small.end());
        EXPECT_EQ(*iter, 1.0);
        *iter = 10.0;
        EXPECT_EQ(*iter, 10.0);

        ++iter;
        ASSERT_NE(iter, small.end());
        EXPECT_EQ(*iter, 2.0);
        *iter = 20.0;
        EXPECT_EQ(*iter, 20.0);

        ++iter;
        ASSERT_NE(iter, small.end());
        EXPECT_EQ(*iter, 3.0);
        *iter = 30.0;
        EXPECT_EQ(*iter, 30.0);

        ++iter;
        EXPECT_EQ(iter, small.end());

        EXPECT_EQ(small[0], 10.0);
        EXPECT_EQ(small[1], 20.0);
        EXPECT_EQ(small[2], 30.0);

    }


    TEST(Utilities_SmallVector, Insert_NoRealloc_Front) {
        SmallVector<double, 5> small{1.0, 2.0};
        std::vector<double> extras{3.0, 4.0};
        small.insert(small.begin(), extras.cbegin(), extras.cend());
        ASSERT_EQ(small.size(), 4);
        ASSERT_GE(small.capacity(), 4);
        EXPECT_EQ(small[0], 3.0);
        EXPECT_EQ(small[1], 4.0);
        EXPECT_EQ(small[2], 1.0);
        EXPECT_EQ(small[3], 2.0);
    }


    TEST(Utilities_SmallVector, Insert_NoRealloc_Middle) {
        SmallVector<double, 5> small{1.0, 2.0};
        std::vector<double> extras{3.0, 4.0};
        small.insert(small.begin() + 1, extras.cbegin(), extras.cend());
        ASSERT_EQ(small.size(), 4);
        ASSERT_GE(small.capacity(), 4);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 3.0);
        EXPECT_EQ(small[2], 4.0);
        EXPECT_EQ(small[3], 2.0);
    }


    TEST(Utilities_SmallVector, Insert_NoRealloc_Back) {
        SmallVector<double, 5> small{1.0, 2.0};
        std::vector<double> extras{3.0, 4.0};
        small.insert(small.end(), extras.cbegin(), extras.cend());
        ASSERT_EQ(small.size(), 4);
        ASSERT_GE(small.capacity(), 4);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
    }

    TEST(Utilities_SmallVector, Insert_Realloc_Front) {
        SmallVector<double, 4> small{1.0, 2.0};
        std::vector<double> extras{3.0, 4.0, 5.0};
        small.insert(small.begin(), extras.cbegin(), extras.cend());
        ASSERT_EQ(small.size(), 5);
        ASSERT_GE(small.capacity(), 5);
        EXPECT_EQ(small[0], 3.0);
        EXPECT_EQ(small[1], 4.0);
        EXPECT_EQ(small[2], 5.0);
        EXPECT_EQ(small[3], 1.0);
        EXPECT_EQ(small[4], 2.0);
    }

    TEST(Utilities_SmallVector, Insert_Realloc_Middle) {
        SmallVector<double, 4> small{1.0, 2.0};
        std::vector<double> extras{3.0, 4.0, 5.0};
        small.insert(small.begin() + 1, extras.cbegin(), extras.cend());
        ASSERT_EQ(small.size(), 5);
        ASSERT_GE(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 3.0);
        EXPECT_EQ(small[2], 4.0);
        EXPECT_EQ(small[3], 5.0);
        EXPECT_EQ(small[4], 2.0);
    }

    TEST(Utilities_SmallVector, Insert_Realloc_Back) {
        SmallVector<double, 4> small{1.0, 2.0};
        std::vector<double> extras{3.0, 4.0, 5.0};
        small.insert(small.end(), extras.cbegin(), extras.cend());
        ASSERT_EQ(small.size(), 5);
        ASSERT_GE(small.capacity(), 5);
        EXPECT_EQ(small[0], 1.0);
        EXPECT_EQ(small[1], 2.0);
        EXPECT_EQ(small[2], 3.0);
        EXPECT_EQ(small[3], 4.0);
        EXPECT_EQ(small[4], 5.0);
    }


    TEST(Utilities_SmallVector, Span) {
        SmallVector<double, 4> small{1.0, 2.0, 3.0};
        auto as_span = static_cast<std::span<double>>(small);
        ASSERT_EQ(as_span.size(), 3);
        EXPECT_EQ(as_span[0], 1.0);
        EXPECT_EQ(as_span[1], 2.0);
        EXPECT_EQ(as_span[2], 3.0);

        // test write
        as_span[1] = 20.0;
        EXPECT_EQ(small[1], 20.0);
    }

    TEST(Utilities_SmallVector, ConstSpan) {
        SmallVector<double, 4> small{1.0, 2.0, 3.0};
        auto as_span = static_cast<std::span<const double>>(small);
        ASSERT_EQ(as_span.size(), 3);
        EXPECT_EQ(as_span[0], 1.0);
        EXPECT_EQ(as_span[1], 2.0);
        EXPECT_EQ(as_span[2], 3.0);
    }

    TEST(Utilities_SmallVector, ConstIterator) {
        SmallVector<double, 4> small{1.0, 2.0, 3.0};

        auto iter = small.cbegin();
        ASSERT_NE(iter, small.cend());
        EXPECT_EQ(*iter, 1.0);

        ++iter;
        ASSERT_NE(iter, small.cend());
        EXPECT_EQ(*iter, 2.0);

        ++iter;
        ASSERT_NE(iter, small.cend());
        EXPECT_EQ(*iter, 3.0);

        ++iter;
        EXPECT_EQ(iter, small.cend());
    }

    /** Test object for counting destructions of non-moved objects */
    struct dtor_test {
        size_t * counter = nullptr;
        dtor_test() noexcept = default;
        explicit dtor_test(size_t& counter) noexcept : counter{&counter} { }

        dtor_test(const dtor_test& rhs) noexcept : counter{rhs.counter} { }

        dtor_test(dtor_test&& rhs) noexcept : counter{rhs.counter} {
            rhs.counter = nullptr;
        }
        dtor_test& operator=(const dtor_test& rhs) noexcept {
            this->counter = rhs.counter;
            return *this;
        }

        dtor_test& operator=(dtor_test&& rhs) noexcept {
            this->counter = rhs.counter;
            rhs.counter = nullptr;
            return *this;
        }

        ~dtor_test() noexcept {
            if (counter) {
                ++(*counter);
            }
        }
    };

    TEST(Utilities_SmallVector, Destructor) {
        size_t total_destructions = 0;
        {
            SmallVector<dtor_test, 3> small{};
            small.emplace_back(total_destructions);
            small.emplace_back(total_destructions);
            small.emplace_back(total_destructions);
            small.emplace_back(total_destructions);
        }
        EXPECT_EQ(total_destructions, 4);
    }

    TEST(Utilities_SmallVector, DestructorAfterMove) {
        size_t total_destructions = 0;
        {
            SmallVector<dtor_test, 3> small{};
            small.emplace_back(total_destructions);
            small.emplace_back(total_destructions);
            small.emplace_back(total_destructions);
            small.emplace_back(total_destructions);

            SmallVector<dtor_test, 3> moved_small(std::move(small));
        }
        EXPECT_EQ(total_destructions, 4);
    }
}