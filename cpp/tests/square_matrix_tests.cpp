/**
 * square_matrix_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"
#include "utilities/square_matrix.h"

namespace NPATK::Tests {

    TEST(SquareMatrix, Empty) {
        SquareMatrix<int> empty{};
        EXPECT_EQ(empty.dimension, 0);
        EXPECT_EQ(empty.begin(), empty.end());

    }

    TEST(SquareMatrix, TwoByTwo) {
        SquareMatrix<int> matrix{2, {1, 2, 3, 4}};
        ASSERT_EQ(matrix.dimension, 2);
        EXPECT_EQ(matrix[0][0], 1);
        EXPECT_EQ(matrix[0][1], 2);
        EXPECT_EQ(matrix[1][0], 3);
        EXPECT_EQ(matrix[1][1], 4);

        auto iter = matrix.begin();
        ASSERT_NE(iter, matrix.end());
        EXPECT_EQ(*iter, 1);
        ++iter;

        ASSERT_NE(iter, matrix.end());
        EXPECT_EQ(*iter, 2);
        ++iter;

        ASSERT_NE(iter, matrix.end());
        EXPECT_EQ(*iter, 3);
        ++iter;

        ASSERT_NE(iter, matrix.end());
        EXPECT_EQ(*iter, 4);
        ++iter;

        EXPECT_EQ(iter, matrix.end());
    }

    TEST(SquareMatrix, TransposeIterator) {
        SquareMatrix<int> matrix{2, {1, 2, 3, 4}};

        ASSERT_EQ(matrix.dimension, 2);
        auto txIter = matrix.ColumnMajor.begin();
        ASSERT_NE(txIter, matrix.ColumnMajor.end());
        EXPECT_EQ(*txIter, 1);
        ++txIter;

        ASSERT_NE(txIter, matrix.ColumnMajor.end());
        EXPECT_EQ(*txIter, 3);
        ++txIter;

        ASSERT_NE(txIter, matrix.ColumnMajor.end());
        EXPECT_EQ(*txIter, 2);
        ++txIter;

        ASSERT_NE(txIter, matrix.ColumnMajor.end());
        EXPECT_EQ(*txIter, 4);
        ++txIter;

        EXPECT_EQ(txIter, matrix.ColumnMajor.end());
    }
}