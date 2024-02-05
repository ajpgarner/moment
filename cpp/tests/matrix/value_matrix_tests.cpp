/**
 * monomial_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "compare_os_matrix.h"
#include "compare_symbol_matrix.h"

#include "matrix/value_matrix.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"


namespace Moment::Tests {
    TEST(Matrix_ValueMatrix, DenseReal) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXd eigen_data{{1.0, 2.0},
                                         {3.0, 4.0}};

        const ValueMatrix matrix{system.Context(), system.Symbols(), system.polynomial_factory().zero_tolerance,
                                eigen_data};


        ASSERT_EQ(matrix.Dimension(), 2);
        EXPECT_FALSE(matrix.Hermitian());
        EXPECT_FALSE(matrix.AntiHermitian());

        EXPECT_EQ(matrix.SymbolMatrix(0, 0), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 1), Monomial(1, 2.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 0), Monomial(1, 3.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 1), Monomial(1, 4.0, false));
    }

    TEST(Matrix_ValueMatrix, DenseReal_WithZeros) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXd eigen_data{{1.0, 0.0},
                                         {0.0, 1.0}};

        const ValueMatrix matrix{system.Context(), system.Symbols(), system.polynomial_factory().zero_tolerance,
                                eigen_data};


        ASSERT_EQ(matrix.Dimension(), 2);
        EXPECT_TRUE(matrix.Hermitian());
        EXPECT_FALSE(matrix.AntiHermitian());

        EXPECT_EQ(matrix.SymbolMatrix(0, 0), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 1), Monomial(0, 0.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 0), Monomial(0, 0.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 1), Monomial(1, 1.0, false));
    }

    TEST(Matrix_ValueMatrix, DenseComplex) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXcd eigen_data{{1.0, {2.0, 1.0}},
                                         {{2.0, -1.0}, 4.0}};

        const ValueMatrix matrix{system.Context(), system.Symbols(), system.polynomial_factory().zero_tolerance,
                                 eigen_data};


        ASSERT_EQ(matrix.Dimension(), 2);
        EXPECT_TRUE(matrix.Hermitian());
        EXPECT_FALSE(matrix.AntiHermitian());

        EXPECT_EQ(matrix.SymbolMatrix(0, 0), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 1), Monomial(1, {2.0, 1.0}, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 0), Monomial(1, {2.0, -1.0}, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 1), Monomial(1, 4.0, false));

    }

    TEST(Matrix_ValueMatrix, SparseReal) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXd eigen_data{{1.0, 0.0, 2.0},
                                         {0.0, 1.0, 3.0},
                                         {2.0, 3.0, 0.0}};
        Eigen::SparseMatrix<double> sparse_data = eigen_data.sparseView();

        const ValueMatrix matrix{system.Context(), system.Symbols(), system.polynomial_factory().zero_tolerance,
                                 sparse_data};

        ASSERT_EQ(matrix.Dimension(), 3);
        EXPECT_TRUE(matrix.Hermitian());
        EXPECT_FALSE(matrix.AntiHermitian());

        EXPECT_EQ(matrix.SymbolMatrix(0, 0), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 1), Monomial(0, 0.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 2), Monomial(1, 2.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 0), Monomial(0, 0.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 1), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 2), Monomial(1, 3.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(2, 0), Monomial(1, 2.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(2, 1), Monomial(1, 3.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(2, 2), Monomial(0, 0.0, false));
    }

    TEST(Matrix_ValueMatrix, SparseComplex) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXcd eigen_data{{1.0, 0.0, {2.0, 1.0}},
                                         {0.0, 1.0, 3.0},
                                         {{2.0, -1.0}, 3.0, 0.0}};
        Eigen::SparseMatrix<std::complex<double>> sparse_data = eigen_data.sparseView();

        const ValueMatrix matrix{system.Context(), system.Symbols(), system.polynomial_factory().zero_tolerance,
                                 sparse_data};

        ASSERT_EQ(matrix.Dimension(), 3);
        EXPECT_TRUE(matrix.Hermitian());
        EXPECT_FALSE(matrix.AntiHermitian());

        EXPECT_EQ(matrix.SymbolMatrix(0, 0), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 1), Monomial(0, 0.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(0, 2), Monomial(1, {2.0, 1.0}, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 0), Monomial(0, 0.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 1), Monomial(1, 1.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(1, 2), Monomial(1, 3.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(2, 0), Monomial(1, {2.0, -1.0}, false));
        EXPECT_EQ(matrix.SymbolMatrix(2, 1), Monomial(1, 3.0, false));
        EXPECT_EQ(matrix.SymbolMatrix(2, 2), Monomial(0, 0.0, false));
    }

    TEST(Matrix_ValueMatrix, PreMultiply_OS_Scalar) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXcd eigen_data{{1.0, {2.0, 1.0}},
                                          {{2.0, -1.0}, 4.0}};

        const ValueMatrix matrix{system.Context(), system.Symbols(),
                                 system.polynomial_factory().zero_tolerance, eigen_data};

        auto res_ptr = matrix.pre_multiply(OperatorSequence::Identity(system.Context()), {2.0, 0.0},
                                           system.polynomial_factory(), system.Symbols(),
                                           Multithreading::MultiThreadPolicy::Never);
        ASSERT_NE(res_ptr, nullptr);
        ASSERT_TRUE(res_ptr->is_monomial());
        const auto& result = dynamic_cast<const MonomialMatrix&>(*res_ptr);
        EXPECT_TRUE(result.Hermitian());
        ASSERT_EQ(result.Dimension(), 2);
        EXPECT_EQ(result.SymbolMatrix(0, 0), Monomial(1, 2.0, false));
        EXPECT_EQ(result.SymbolMatrix(0, 1), Monomial(1, {4.0, 2.0}, false));
        EXPECT_EQ(result.SymbolMatrix(1, 0), Monomial(1, {4.0, -2.0}, false));
        EXPECT_EQ(result.SymbolMatrix(1, 1), Monomial(1, 8.0, false));
    }

    TEST(Matrix_ValueMatrix, PostMultiply_OS_Hermitian) {
        MatrixSystem system{std::make_unique<Context>(2)};

        const Eigen::MatrixXcd eigen_data{{1.0, {2.0, 1.0}},
                                          {{2.0, -1.0}, 4.0}};

        const ValueMatrix matrix{system.Context(), system.Symbols(),
                                 system.polynomial_factory().zero_tolerance, eigen_data};

        auto res_ptr = matrix.post_multiply(OperatorSequence{{1,1}, system.Context()}, {1.0, 0.0},
                                           system.polynomial_factory(), system.Symbols(),
                                           Multithreading::MultiThreadPolicy::Never);
        ASSERT_NE(res_ptr, nullptr);
        ASSERT_TRUE(res_ptr->is_monomial());
        const auto& result = dynamic_cast<const MonomialMatrix&>(*res_ptr);

        auto where_xx = system.Symbols().where(OperatorSequence{{1,1}, system.Context()});
        ASSERT_TRUE(where_xx.found()) << system.Symbols();
        auto sXX = where_xx->Id();

        EXPECT_TRUE(result.Hermitian());
        ASSERT_EQ(result.Dimension(), 2);
        EXPECT_EQ(result.SymbolMatrix(0, 0), Monomial(sXX, 1.0, false));
        EXPECT_EQ(result.SymbolMatrix(0, 1), Monomial(sXX, {2.0, 1.0}, false));
        EXPECT_EQ(result.SymbolMatrix(1, 0), Monomial(sXX, {2.0, -1.0}, false));
        EXPECT_EQ(result.SymbolMatrix(1, 1), Monomial(sXX, 4.0, false));
    }
}