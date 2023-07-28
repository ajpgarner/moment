/**
 * square_matrix_tests.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"
#include "tensor/square_matrix.h"

namespace Moment::Tests {

    TEST(Tensor_SquareMatrix, Empty) {
        SquareMatrix<int> empty{};
        EXPECT_EQ(empty.dimension, 0);
        EXPECT_EQ(empty.begin(), empty.end());

    }

    TEST(Tensor_SquareMatrix, TwoByTwo) {
        SquareMatrix<int> matrix{2, {1, 2, 3, 4}}; // = [1 3; 2 4] in CM order
        ASSERT_EQ(matrix.dimension, 2);
        EXPECT_EQ(matrix(0, 0), 1);
        EXPECT_EQ(matrix(0, 1), 3);
        EXPECT_EQ(matrix(1, 0), 2);
        EXPECT_EQ(matrix(1, 1), 4);

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

    TEST(Tensor_SquareMatrix, TransposeIterator) {
        SquareMatrix<int> matrix{2, {1, 2, 3, 4}};

        ASSERT_EQ(matrix.dimension, 2);
        auto txIter = matrix.Transpose.begin();
        ASSERT_NE(txIter, matrix.Transpose.end());
        EXPECT_EQ(*txIter, 1);
        ++txIter;

        ASSERT_NE(txIter, matrix.Transpose.end());
        EXPECT_EQ(*txIter, 3);
        ++txIter;

        ASSERT_NE(txIter, matrix.Transpose.end());
        EXPECT_EQ(*txIter, 2);
        ++txIter;

        ASSERT_NE(txIter, matrix.Transpose.end());
        EXPECT_EQ(*txIter, 4);
        ++txIter;

        EXPECT_EQ(txIter, matrix.Transpose.end());
    }


    TEST(Tensor_SquareMatrix, Pad) {
        SquareMatrix<int> matrix{2, {1, 2, 3, 4}};
        ASSERT_EQ(matrix.dimension, 2);
        ASSERT_EQ(matrix(0, 0), 1);
        ASSERT_EQ(matrix(0, 1), 3);
        ASSERT_EQ(matrix(1, 0), 2);
        ASSERT_EQ(matrix(1, 1), 4);

        auto padded_matrix = matrix.pad(2, 0);
        ASSERT_EQ(padded_matrix.dimension, 4);
        EXPECT_EQ(padded_matrix(0, 0), 1);
        EXPECT_EQ(padded_matrix(0, 1), 3);
        EXPECT_EQ(padded_matrix(0, 2), 0);
        EXPECT_EQ(padded_matrix(0, 3), 0);
        EXPECT_EQ(padded_matrix(1, 0), 2);
        EXPECT_EQ(padded_matrix(1, 1), 4);
        EXPECT_EQ(padded_matrix(1, 2), 0);
        EXPECT_EQ(padded_matrix(1, 3), 0);
        EXPECT_EQ(padded_matrix(2, 1), 0);
        EXPECT_EQ(padded_matrix(2, 2), 0);
        EXPECT_EQ(padded_matrix(2, 2), 0);
        EXPECT_EQ(padded_matrix(2, 3), 0);
        EXPECT_EQ(padded_matrix(3, 1), 0);
        EXPECT_EQ(padded_matrix(3, 2), 0);
        EXPECT_EQ(padded_matrix(3, 2), 0);
        EXPECT_EQ(padded_matrix(3, 3), 0);
    }

    TEST(Tensor_SquareMatrix, Triangle_LowerInclusiveConst) {
        const SquareMatrix<int> matrix{2, {1, 2, 3, 4}}; // col-major data <- [1 3; 2 4]

        auto range = matrix.LowerTriangle(); // 1, 2, 4
        auto iter = range.begin();
        const auto iter_end = range.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 0);
        EXPECT_EQ(iter.Index()[1], 0);
        EXPECT_EQ(iter.Offset(), 0);
        EXPECT_EQ(*iter, 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 1);
        EXPECT_EQ(iter.Index()[1], 0);
        EXPECT_EQ(iter.Offset(), 1);
        EXPECT_EQ(*iter, 2);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 1);
        EXPECT_EQ(iter.Index()[1], 1);
        EXPECT_EQ(iter.Offset(), 3);
        EXPECT_EQ(*iter, 4);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_SquareMatrix, Triangle_LowerInclusiveConst_Empty) {
        const SquareMatrix<int> matrix{0, {}};
        auto range = matrix.LowerTriangle();
        auto iter = range.begin();
        const auto iter_end = range.end();
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_SquareMatrix, Triangle_UpperInclusiveConst) {
        const SquareMatrix<int> matrix{2, {1, 2, 3, 4}}; // col-major data <- [1 3; 2 4]

        auto range = matrix.UpperTriangle(); // 1, 3, 4
        auto iter = range.begin();
        const auto iter_end = range.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 0);
        EXPECT_EQ(iter.Index()[1], 0);
        EXPECT_EQ(iter.Offset(), 0);
        EXPECT_EQ(*iter, 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 0);
        EXPECT_EQ(iter.Index()[1], 1);
        EXPECT_EQ(iter.Offset(), 2);
        EXPECT_EQ(*iter, 3);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 1);
        EXPECT_EQ(iter.Index()[1], 1);
        EXPECT_EQ(iter.Offset(), 3);
        EXPECT_EQ(*iter, 4);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_SquareMatrix, Triangle_UpperInclusiveConst_Empty) {
        const SquareMatrix<int> matrix{0, {}};
        auto range = matrix.UpperTriangle();
        auto iter = range.begin();
        const auto iter_end = range.end();
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_SquareMatrix, Triangle_LowerExclusiveConst) {
        const SquareMatrix<int> matrix{3, {1, 2, 3, 4, 5, 6, 7, 8, 9}};
        // 1 4 7
        // 2 5 8
        // 3 6 9

        auto range = matrix.ExclusiveLowerTriangle(); // 2 3 6
        auto iter = range.begin();
        const auto iter_end = range.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 1);
        EXPECT_EQ(iter.Index()[1], 0);
        EXPECT_EQ(iter.Offset(), 1);
        EXPECT_EQ(*iter, 2);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 2);
        EXPECT_EQ(iter.Index()[1], 0);
        EXPECT_EQ(iter.Offset(), 2);
        EXPECT_EQ(*iter, 3);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 2);
        EXPECT_EQ(iter.Index()[1], 1);
        EXPECT_EQ(iter.Offset(), 5);
        EXPECT_EQ(*iter, 6);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_SquareMatrix, Triangle_LowerExclusiveConst_Empty) {
        const SquareMatrix<int> matrix{0, {}};
        auto range = matrix.ExclusiveLowerTriangle();
        auto iter = range.begin();
        const auto iter_end = range.end();
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_SquareMatrix, Triangle_LowerExclusiveConst_Scalar) {
        const SquareMatrix<int> matrix{1, {1}};
        auto range = matrix.ExclusiveLowerTriangle();
        auto iter = range.begin();
        const auto iter_end = range.end();
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_SquareMatrix, Triangle_UpperExclusiveConst) {
        const SquareMatrix<int> matrix{3, {1, 2, 3, 4, 5, 6, 7, 8, 9}};
        // 1 4 7
        // 2 5 8
        // 3 6 9

        auto range = matrix.ExclusiveUpperTriangle(); // 4 7 8
        auto iter = range.begin();
        const auto iter_end = range.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 0);
        EXPECT_EQ(iter.Index()[1], 1);
        EXPECT_EQ(iter.Offset(), 3);
        EXPECT_EQ(*iter, 4);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 0);
        EXPECT_EQ(iter.Index()[1], 2);
        EXPECT_EQ(iter.Offset(), 6);
        EXPECT_EQ(*iter, 7);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(iter.Index()[0], 1);
        EXPECT_EQ(iter.Index()[1], 2);
        EXPECT_EQ(iter.Offset(), 7);
        EXPECT_EQ(*iter, 8);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Tensor_SquareMatrix, Triangle_UpperExclusiveConst_Empty) {
        const SquareMatrix<int> matrix{0, {}};
        auto range = matrix.ExclusiveUpperTriangle();
        auto iter = range.begin();
        const auto iter_end = range.end();
        EXPECT_EQ(iter, iter_end);
    }


    TEST(Tensor_SquareMatrix, Triangle_UpperExclusiveConst_Scalar) {
        const SquareMatrix<int> matrix{1, {1}};
        auto range = matrix.ExclusiveUpperTriangle();
        auto iter = range.begin();
        const auto iter_end = range.end();
        EXPECT_EQ(iter, iter_end);
    }
}