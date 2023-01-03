/**
 * square_matrix_tests.cpp
 * 
 * Copyright (c) 2022-2023 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"
#include "utilities/square_matrix.h"

namespace Moment::Tests {

    TEST(Utilities_SquareMatrix, Empty) {
        SquareMatrix<int> empty{};
        EXPECT_EQ(empty.dimension, 0);
        EXPECT_EQ(empty.begin(), empty.end());

    }

    TEST(Utilities_SquareMatrix, TwoByTwo) {
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

    TEST(Utilities_SquareMatrix, TransposeIterator) {
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


    TEST(Utilities_SquareMatrix, Pad) {
        SquareMatrix<int> matrix{2, {1, 2, 3, 4}};
        ASSERT_EQ(matrix.dimension, 2);
        ASSERT_EQ(matrix[0][0], 1);
        ASSERT_EQ(matrix[0][1], 2);
        ASSERT_EQ(matrix[1][0], 3);
        ASSERT_EQ(matrix[1][1], 4);

        auto padded_matrix = matrix.pad(2, 0);
        ASSERT_EQ(padded_matrix.dimension, 4);
        EXPECT_EQ(padded_matrix[0][0], 1);
        EXPECT_EQ(padded_matrix[0][1], 2);
        EXPECT_EQ(padded_matrix[0][2], 0);
        EXPECT_EQ(padded_matrix[0][3], 0);
        EXPECT_EQ(padded_matrix[1][0], 3);
        EXPECT_EQ(padded_matrix[1][1], 4);
        EXPECT_EQ(padded_matrix[1][2], 0);
        EXPECT_EQ(padded_matrix[1][3], 0);
        EXPECT_EQ(padded_matrix[2][1], 0);
        EXPECT_EQ(padded_matrix[2][2], 0);
        EXPECT_EQ(padded_matrix[2][2], 0);
        EXPECT_EQ(padded_matrix[2][3], 0);
        EXPECT_EQ(padded_matrix[3][1], 0);
        EXPECT_EQ(padded_matrix[3][2], 0);
        EXPECT_EQ(padded_matrix[3][2], 0);
        EXPECT_EQ(padded_matrix[3][3], 0);



    }
}