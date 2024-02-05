/**
 * eigen_utils_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/eigen_utils.h"
#include "utilities/float_utils.h"

namespace Moment::Tests {
    TEST(Utilities_EigenUtils, IsZero_DenseReal) {

        Eigen::MatrixXd test_non_zero{{1.0, 2.0, 3.0},
                                   {4.0, 5.0, 6.0},
                                   {7.0, 8.0, 9.0}};

        EXPECT_FALSE(is_zero(test_non_zero, 1.0));

        Eigen::MatrixXd test_zero{{0.0, 0.0},
                                  {0.0, 0.0}};

        EXPECT_TRUE(is_zero(test_zero, 1.0));
    }

    TEST(Utilities_EigenUtils, IsZero_DenseComplex) {

        Eigen::MatrixXcd test_non_zero{{{1.0,0.0}, 2.0, 3.0},
                                       {4.0, 5.0, 6.0},
                                       {7.0, 8.0, 9.0}};

        EXPECT_FALSE(is_zero(test_non_zero, 1.0));

        Eigen::MatrixXcd test_zero{{0.0, 0.0},
                                  {0.0, 0.0}};

        EXPECT_TRUE(is_zero(test_zero, 1.0));
    }

    TEST(Utilities_EigenUtils, IsZero_SparseReal) {
        Eigen::MatrixXd test_non_zero{{1.0, 2.0, 3.0},
                                      {4.0, 5.0, 6.0},
                                      {7.0, 8.0, 9.0}};
        Eigen::SparseMatrix<double> test_non_zero_sparse = test_non_zero.sparseView();

        EXPECT_FALSE(is_zero(test_non_zero_sparse, 1.0));

        Eigen::MatrixXd test_zero{{0.0, 0.0},
                                  {0.0, 0.0}};
        Eigen::SparseMatrix<double> test_zero_sparse = test_zero.sparseView();

        EXPECT_TRUE(is_zero(test_zero_sparse, 1.0));
    }


    TEST(Utilities_EigenUtils, IsZero_SparseComplex) {


        Eigen::MatrixXcd test_non_zero{{{1.0,0.0}, 2.0, 3.0},
                                       {4.0, 5.0, 6.0},
                                       {7.0, 8.0, 9.0}};
        Eigen::SparseMatrix<std::complex<double>> test_non_zero_sparse = test_non_zero.sparseView();

        EXPECT_FALSE(is_zero(test_non_zero_sparse, 1.0));


        Eigen::MatrixXcd test_zero{{0.0, 0.0},
                                   {0.0, 0.0}};
        Eigen::SparseMatrix<std::complex<double>> test_zero_sparse = test_zero.sparseView();
        EXPECT_TRUE(is_zero(test_zero_sparse, 1.0));
    }

    TEST(Utilities_EigenUtils, IsHermitian_DenseReal) {

        Eigen::MatrixXd test_non_H{{1.0, 2.0, 3.0},
                                   {4.0, 5.0, 6.0},
                                   {7.0, 8.0, 9.0}};

        EXPECT_FALSE(is_hermitian(test_non_H, 1.0));

        Eigen::MatrixXd test_H{{1.0, 2.0, 3.0},
                               {2.0, 5.0, 6.0},
                               {3.0, 6.0, 9.0}};

        EXPECT_TRUE(is_hermitian(test_H, 1.0));
    }

    TEST(Utilities_EigenUtils, IsHermitian_DenseComplex) {

        Eigen::MatrixXcd test_non_H1{{1.0, 2.0, 3.0},
                                     {4.0, 5.0, {6.0, 1.0}},
                                     {7.0, 8.0, 9.0}};

        EXPECT_FALSE(is_hermitian(test_non_H1, 1.0));

        Eigen::MatrixXcd test_non_H2{{1.0, 0.0, 0.0},
                                     {0.0, 5.0, 0.0},
                                     {0.0, 0.0, {0.0, 9.0}}};

        EXPECT_FALSE(is_hermitian(test_non_H2, 1.0));

        Eigen::MatrixXcd test_H{{1.0, {2.0, 1.0}, 3.0},
                                {{2.0, -1.0}, 5.0, {6.0, -2.0}},
                                {3.0, {6.0, 2.0}, 9.0}};

        EXPECT_TRUE(is_hermitian(test_H, 1.0));
    }

    TEST(Utilities_EigenUtils, IsHermitian_SparseReal) {

        Eigen::MatrixXd test_non_H{{1.0, 2.0, 3.0},
                                   {4.0, 5.0, 6.0},
                                   {7.0, 8.0, 9.0}};
        Eigen::SparseMatrix<double> sparse_test_non_H = test_non_H.sparseView();

        EXPECT_FALSE(is_hermitian(sparse_test_non_H, 1.0));

        Eigen::MatrixXd test_H{{1.0, 2.0, 3.0},
                               {2.0, 5.0, 6.0},
                               {3.0, 6.0, 9.0}};
        Eigen::SparseMatrix<double> sparse_test_H = test_H.sparseView();

        EXPECT_TRUE(is_hermitian(sparse_test_H, 1.0));
    }


    TEST(Utilities_EigenUtils, IsHermitian_SparseComplex) {

        Eigen::MatrixXcd test_non_H1{{1.0, 2.0, 3.0},
                                     {4.0, 5.0, {6.0, 1.0}},
                                     {7.0, 8.0, 9.0}};

        Eigen::SparseMatrix<std::complex<double>> sparse_test_non_H1 = test_non_H1.sparseView();
        EXPECT_FALSE(is_hermitian(sparse_test_non_H1, 1.0));

        Eigen::MatrixXcd test_non_H2{{1.0, 0.0, 0.0},
                                     {0.0, 5.0, 0.0},
                                     {0.0, 0.0, {0.0, 9.0}}};

        Eigen::SparseMatrix<std::complex<double>> sparse_test_non_H2 = test_non_H2.sparseView();
        EXPECT_FALSE(is_hermitian(sparse_test_non_H2, 1.0));

        Eigen::MatrixXcd test_H{{1.0, {2.0, 1.0}, 3.0},
                                {{2.0, -1.0}, 5.0, {6.0, -2.0}},
                                {3.0, {6.0, 2.0}, 9.0}};

        Eigen::SparseMatrix<std::complex<double>> sparse_test_H = test_H.sparseView();
        EXPECT_TRUE(is_hermitian(sparse_test_H, 1.0));
    }

    TEST(Utilities_EigenUtils, IsAntiHermitian_DenseComplex) {

        Eigen::MatrixXcd test_non_AH{{1.0, {2.0, 1.0}, 3.0},
                                     {{2.0, -1.0}, 5.0, {6.0, -2.0}},
                                     {3.0, {6.0, 2.0}, 9.0}};

        EXPECT_FALSE(is_antihermitian(test_non_AH, 1.0));

        Eigen::MatrixXcd test_AH{{{0.0, 1.0}, {3.0, 2.0}},
                                 {{-3.0, 2.0}, 0.0}};
        EXPECT_TRUE(is_antihermitian(test_AH, 1.0));

    }

    TEST(Utilities_EigenUtils, IsAntiHermitian_SparseComplex) {
        Eigen::MatrixXcd test_non_AH{{1.0, {2.0, 1.0}, 3.0},
                                     {{2.0, -1.0}, 5.0, {6.0, -2.0}},
                                     {3.0, {6.0, 2.0}, 9.0}};

        Eigen::SparseMatrix<std::complex<double>> sparse_test_non_AH = test_non_AH.sparseView();
        EXPECT_FALSE(is_antihermitian(sparse_test_non_AH, 1.0));

        Eigen::MatrixXcd test_AH{{{0.0, 1.0}, {3.0, 2.0}},
                                 {{-3.0, 2.0}, 0.0}};
        Eigen::SparseMatrix<std::complex<double>> sparse_test_AH = test_AH.sparseView();
        EXPECT_TRUE(is_antihermitian(test_AH, 1.0));
    }

}