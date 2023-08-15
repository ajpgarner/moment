/**
 * triangular_index_iterator_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "utilities/triangular_index_iterator.h"

namespace Moment::Tests {

    namespace {
        template<typename t_iter_t>
        void assert_same_index(const t_iter_t& iter, const t_iter_t& iter_end,
                               size_t expected_index, std::initializer_list<size_t> expected_values) {
            ASSERT_TRUE(!iter_end) << "Global = " << expected_index;
            ASSERT_NE(iter, iter_end) << "Global = " << expected_index;
            EXPECT_TRUE(iter) << "Global = " << expected_index;
            EXPECT_FALSE(!iter) << "Global = " << expected_index;
            EXPECT_EQ(iter.global(), expected_index) << "Global = " << expected_index;
            const auto& actual_values = *iter;
            ASSERT_EQ(actual_values.size(), expected_values.size()) << "Global = " << expected_index;

            size_t sub_idx = 0;
            for (auto expected_value : expected_values) {
                EXPECT_EQ(actual_values[sub_idx], expected_value)
                    << "Global = " << expected_index << ", index = " << sub_idx;
                ++sub_idx;
            }
        }
    }

    TEST(Utilities_TriangularIterator, LengthZero_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{1, 0};
        DuplicateTriangularIndexIterator<> mdiIterEnd{1, 0, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, LengthZero_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{1, 0};
        UniqueTriangularIndexIterator<> mdiIterEnd{1, 0, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, Empty_LengthZero_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{0, 0};
        DuplicateTriangularIndexIterator<> mdiIterEnd{0, 0, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, Empty_LengthZero_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{0, 0};
        UniqueTriangularIndexIterator<> mdiIterEnd{0, 0, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, Empty_LengthOne_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{0, 1};
        DuplicateTriangularIndexIterator<> mdiIterEnd{0, 1, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, Empty_LengthOne_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{0, 1};
        UniqueTriangularIndexIterator<> mdiIterEnd{0, 1, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, Empty_LengthTwo_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{0, 2};
        DuplicateTriangularIndexIterator<> mdiIterEnd{0, 2, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, Empty_LengthTwo_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{0, 2};
        UniqueTriangularIndexIterator<> mdiIterEnd{0, 2, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }


    TEST(Utilities_TriangularIterator, OneDimensional_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{4, 1};
        DuplicateTriangularIndexIterator<> mdiIterEnd{4, 1, true};

        assert_same_index(mdiIter, mdiIterEnd, 0, {0});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 1, {1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 2, {2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 3, {3});

        ++mdiIter;
        EXPECT_FALSE(mdiIter);
        EXPECT_EQ(mdiIter, mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, OneDimensional_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{4, 1};
        UniqueTriangularIndexIterator<> mdiIterEnd{4, 1, true};

        assert_same_index(mdiIter, mdiIterEnd, 0, {0});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 1, {1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 2, {2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 3, {3});

        ++mdiIter;
        EXPECT_FALSE(mdiIter);
        EXPECT_EQ(mdiIter, mdiIterEnd);
    }



    TEST(Utilities_TriangularIterator, TwoDimensional_ThreeSymbols_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{3, 2};
        DuplicateTriangularIndexIterator<> mdiIterEnd{3, 2, true};

        assert_same_index(mdiIter, mdiIterEnd, 0, {0, 0});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 1, {0, 1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 2, {0, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 3, {1, 1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 4, {1, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 5, {2, 2});

        ++mdiIter;
        EXPECT_FALSE(mdiIter);
        EXPECT_EQ(mdiIter, mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, TwoDimensional_ThreeSymbols_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{3, 2};
        UniqueTriangularIndexIterator<> mdiIterEnd{3, 2, true};

        assert_same_index(mdiIter, mdiIterEnd, 0, {0, 1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 1, {0, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 2, {1, 2});

        ++mdiIter;
        EXPECT_FALSE(mdiIter);
        EXPECT_EQ(mdiIter, mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, ThreeDimensional_ThreeSymbols_Duplicate) {
        DuplicateTriangularIndexIterator<> mdiIter{3, 3};
        DuplicateTriangularIndexIterator<> mdiIterEnd{3, 3, true};

        assert_same_index(mdiIter, mdiIterEnd, 0, {0, 0, 0});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 1, {0, 0, 1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 2, {0, 0, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 3, {0, 1, 1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 4, {0, 1, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 5, {0, 2, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 6, {1, 1, 1});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 7, {1, 1, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 8, {1, 2, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 9, {2, 2, 2});

        ++mdiIter;
        EXPECT_FALSE(mdiIter);
        EXPECT_EQ(mdiIter, mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, ThreeDimensional_FiveSymbols_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{5, 3};
        UniqueTriangularIndexIterator<> mdiIterEnd{5, 3, true};

        assert_same_index(mdiIter, mdiIterEnd, 0, {0, 1, 2});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 1, {0, 1, 3});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 2, {0, 1, 4});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 3, {0, 2, 3});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 4, {0, 2, 4});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 5, {0, 3, 4});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 6, {1, 2, 3});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 7, {1, 2, 4});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 8, {1, 3, 4});

        ++mdiIter;
        assert_same_index(mdiIter, mdiIterEnd, 9, {2, 3, 4});

        ++mdiIter;
        EXPECT_FALSE(mdiIter);
        EXPECT_EQ(mdiIter, mdiIterEnd);
    }

    TEST(Utilities_TriangularIterator, TooLong_Unique) {
        UniqueTriangularIndexIterator<> mdiIter{3, 4};
        UniqueTriangularIndexIterator<> mdiIterEnd{3, 4, true};

        EXPECT_FALSE(mdiIter);
        EXPECT_TRUE(!mdiIter);
        EXPECT_TRUE(mdiIter == mdiIterEnd);
        EXPECT_FALSE(mdiIter != mdiIterEnd);
    }
}