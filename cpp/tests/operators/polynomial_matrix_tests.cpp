/**
 * polynomial_matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "matrix/polynomial_matrix.h"
#include "scenarios/imported/imported_matrix_system.h"
#include "symbolic/symbol_table.h"

#include "compare_basis.h"

#include <string>

namespace Moment::Tests {
    namespace {

        std::pair<std::unique_ptr<Imported::ImportedMatrixSystem>,std::unique_ptr<PolynomialMatrix>>
        stage() {
            auto system = std::make_unique<Imported::ImportedMatrixSystem>();
            auto& symbols = system->Symbols();
            symbols.create(true, false); // 2, real
            symbols.create(true, true); // 3, complex
            symbols.create(true, true); // 4, complex
            symbols.create(true, false); // 5, real

            // Now, create matrix
            std::vector<SymbolCombo> smData;
            smData.reserve(4);

            smData.emplace_back(SymbolCombo{SymbolExpression{1, 1.0}, SymbolExpression{2, -1.0}});
            smData.emplace_back(SymbolCombo{SymbolExpression{3, 1.0}, SymbolExpression{4, 2.0}});
            smData.emplace_back(SymbolCombo{SymbolExpression{3, 1.0, true}, SymbolExpression{4, 2.0, true}});
            smData.emplace_back(SymbolCombo{SymbolExpression{5, 1.0}});

            std::unique_ptr<SquareMatrix<SymbolCombo>> sqMat = std::make_unique<SquareMatrix<SymbolCombo>>(2, std::move(smData));
            auto matrix = std::make_unique<PolynomialMatrix>(system->Context(), system->Symbols(), std::move(sqMat));

            return {std::move(system), std::move(matrix)};
        }

        std::pair<std::vector<dense_real_elem_t>, std::vector<dense_complex_elem_t>>
        reference_dense() {
            std::pair<std::vector<dense_real_elem_t>, std::vector<dense_complex_elem_t>> output
                    = std::make_pair(std::vector<dense_real_elem_t>(5, dense_real_elem_t::Zero(2, 2)),
                                     std::vector<dense_complex_elem_t>(2, dense_complex_elem_t::Zero(2, 2)));

            auto& real = output.first;
            auto& im = output.second;

            real[0](0, 0) = 1.0;

            real[1](0, 0) = -1.0;

            real[2](0, 1) = 1.0;
            real[2](1, 0) = 1.0;

            real[3](0, 1) = 2.0;
            real[3](1, 0) = 2.0;

            real[4](1, 1) = 1.0;

            im[0](0, 1) = std::complex(0.0, 1.0);
            im[0](1, 0) = std::complex(0.0, -1.0);

            im[1](0, 1) = std::complex(0.0, 2.0);
            im[1](1, 0) = std::complex(0.0, -2.0);

            return output;
        }

        std::pair<dense_real_elem_t, dense_complex_elem_t>
        reference_dense_monolithic() {
            std:std::pair<dense_real_elem_t, dense_complex_elem_t> output
                = std::make_pair(dense_real_elem_t::Zero(4, 5), dense_complex_elem_t::Zero(4, 2));

            auto& real = output.first;
            auto& im = output.second;

            real(0, 0) = 1.0;

            real(0, 1) = -1.0;

            real(1, 2) = 1.0;
            real(2, 2) = 1.0; // col maj-> 1*2+0 = 2

            real(1, 3) = 2.0;
            real(2, 3) = 2.0;

            real(3, 4) = 1.0;

            im(2, 0) = std::complex(0.0, 1.0);
            im(1, 0) = std::complex(0.0, -1.0);

            im(2, 1) = std::complex(0.0, 2.0);
            im(1, 1) = std::complex(0.0, -2.0);

            return output;
        }

        std::pair<std::vector<sparse_real_elem_t>, std::vector<sparse_complex_elem_t>> reference_sparse() {
            std::pair<std::vector<sparse_real_elem_t>, std::vector<sparse_complex_elem_t>> output;

            auto& real = output.first;
            auto& im = output.second;

            auto [dense_re, dense_im] = reference_dense();
            for (const auto& b : dense_re) {
                real.emplace_back(b.sparseView());
            }
            for (const auto& b : dense_im) {
                im.emplace_back(b.sparseView());
            }

            return output;
        }

        std::pair<sparse_real_elem_t, sparse_complex_elem_t> reference_sparse_monolithic() {
            std::pair<sparse_real_elem_t , sparse_complex_elem_t> output
                    = std::make_pair(sparse_real_elem_t(4,5), sparse_complex_elem_t(4,2));

            auto& real = output.first;
            real.setZero();
            real.insert(0, 0) = 1.0;
            real.insert(0, 1) = -1.0;
            real.insert(1, 2) = 1.0;
            real.insert(2, 2) = 1.0;
            real.insert(1, 3) = 2.0;
            real.insert(2, 3) = 2.0;
            real.insert(3, 4) = 1.0;
            real.makeCompressed();


            auto& im = output.second;
            im.setZero();
            im.insert(2, 0) = std::complex(0.0, 1.0);
            im.insert(1, 0) = std::complex(0.0, -1.0);
            im.insert(2, 1) = std::complex(0.0, 2.0);
            im.insert(1, 1) = std::complex(0.0, -2.0);
            im.makeCompressed();

            return output;
        }


    }

    TEST(Operators_PolynomialMatrix, Construct) {
        auto [imsPtr, matPtr] = stage();
        ASSERT_TRUE(imsPtr);
        ASSERT_TRUE(matPtr);
        ASSERT_EQ(imsPtr->Symbols().size(), 6);
        ASSERT_EQ(matPtr->Dimension(), 2);
        const auto& matrix = *matPtr;

        const auto& elem00 = matrix.SymbolMatrix[0][0];
        ASSERT_EQ(elem00.size(), 2);
        EXPECT_EQ(elem00[0], SymbolExpression(1, 1.0, false));
        EXPECT_EQ(elem00[1], SymbolExpression(2, -1.0, false));

        const auto& elem01 = matrix.SymbolMatrix[0][1];
        ASSERT_EQ(elem01.size(), 2);
        EXPECT_EQ(elem01[0], SymbolExpression(3, 1.0, false));
        EXPECT_EQ(elem01[1], SymbolExpression(4, 2.0, false));

        const auto& elem10 = matrix.SymbolMatrix[1][0];
        ASSERT_EQ(elem10.size(), 2);
        EXPECT_EQ(elem10[0], SymbolExpression(3, 1.0, true));
        EXPECT_EQ(elem10[1], SymbolExpression(4, 2.0, true));

        const auto& elem11 = matrix.SymbolMatrix[1][1];
        ASSERT_EQ(elem11.size(), 1);
        EXPECT_EQ(elem11[0], SymbolExpression(5, 1.0, false));

        EXPECT_TRUE(matrix.is_hermitian());
        const auto& smp = matrix.SMP();
        EXPECT_TRUE(smp.IsHermitian());
        EXPECT_TRUE(smp.IsComplex());
    }

    TEST(Operators_PolynomialMatrix, DenseBasis) {
        auto [imsPtr, matPtr] = stage();
        ASSERT_TRUE(imsPtr);

        const auto [real, imaginary]  = matPtr->Basis.Dense();

        const auto [ref_real, ref_imaginary] = reference_dense();

        assert_same_basis("Real", real, ref_real);
        assert_same_basis("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_PolynomialMatrix, DenseMonolithicBasis) {
        auto [imsPtr, matPtr] = stage();
        ASSERT_TRUE(imsPtr);

        const auto [real, imaginary] = matPtr->Basis.DenseMonolithic();

        const auto [ref_real, ref_imaginary] = reference_dense_monolithic();

        assert_same_matrix("Real", real, ref_real);
        assert_same_matrix("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_PolynomialMatrix, SparseBasis) {
        auto [imsPtr, matPtr] = stage();
        ASSERT_TRUE(imsPtr);

        const auto [real, imaginary] = matPtr->Basis.Sparse();

        const auto [ref_real, ref_imaginary] = reference_sparse();

        assert_same_basis("Real", real, ref_real);
        assert_same_basis("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_PolynomialMatrix, SparseMonolithicBasis) {
        auto [imsPtr, matPtr] = stage();
        ASSERT_TRUE(imsPtr);

        auto [real, imaginary] = matPtr->Basis.SparseMonolithic();

        const auto [ref_real, ref_imaginary] = reference_sparse_monolithic();

        assert_same_matrix("Real", real, ref_real);
        assert_same_matrix("Imaginary", imaginary, ref_imaginary);
    }
}