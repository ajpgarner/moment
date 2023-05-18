/**
 * matrix_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_matrix_system.h"
#include "scenarios/algebraic/algebraic_context.h"

#include "matrix/moment_matrix.h"

#include "compare_basis.h"

#include <memory>
#include <string>

namespace Moment::Tests {

    namespace {

        using dense_real_elem_t = Eigen::MatrixXd;
        using dense_complex_elem_t =  Eigen::MatrixXcd;
        using sparse_real_elem_t = Eigen::SparseMatrix<double>;
        using sparse_complex_elem_t = Eigen::SparseMatrix<std::complex<double>>;

        std::pair<std::vector<dense_real_elem_t>, std::vector<dense_complex_elem_t>> reference_dense() {
            std::pair<std::vector<dense_real_elem_t>, std::vector<dense_complex_elem_t>> output
                = std::make_pair(std::vector<dense_real_elem_t>(6, dense_real_elem_t::Zero(3, 3)),
                                 std::vector<dense_complex_elem_t>(1, dense_complex_elem_t::Zero(3, 3)));

            auto& real = output.first;
            auto& im = output.second;

            real[0](0, 0) = 1.0;

            real[1](0, 1) = 1.0;
            real[1](1, 0) = 1.0;

            real[2](0, 2) = 1.0;
            real[2](2, 0) = 1.0;

            real[3](1, 1) = 1.0;

            real[4](1, 2) = 1.0;
            real[4](2, 1) = 1.0;

            real[5](2, 2) = 1.0;

            im[0](1, 2) = std::complex(0.0, 1.0);
            im[0](2, 1) = std::complex(0.0, -1.0);

            return output;
        }

        std::pair<dense_real_elem_t, dense_complex_elem_t> reference_dense_monolithic() {
            std:std::pair<dense_real_elem_t, dense_complex_elem_t> output
                = std::make_pair(dense_real_elem_t::Zero(9, 6), dense_complex_elem_t::Zero(9, 1));

            auto& real = output.first;
            auto& im = output.second;

            real(0, 0) = 1.0;

            real(1, 1) = 1.0;
            real(3, 1) = 1.0;

            real(2, 2) = 1.0;
            real(6, 2) = 1.0;

            real(4, 3) = 1.0;

            real(5, 4) = 1.0;
            real(7, 4) = 1.0;

            real(8, 5) = 1.0;

            im(7, 0) = std::complex(0.0, 1.0); // (1,2) -> 2*3+1=7 (col major!)
            im(5, 0) = std::complex(0.0, -1.0); // (2,1) -> 1*3+2=5 (col major!)

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
                = std::make_pair(sparse_real_elem_t(9,6), sparse_complex_elem_t(9,1));

            auto& real = output.first;
            real.setZero();
            real.insert(0, 0) = 1.0;
            real.insert(1, 1) = 1.0;
            real.insert(3, 1) = 1.0;
            real.insert(2, 2) = 1.0;
            real.insert(6, 2) = 1.0;
            real.insert(4, 3) = 1.0;
            real.insert(5, 4) = 1.0;
            real.insert(7, 4) = 1.0;
            real.insert(8, 5) = 1.0;
            real.makeCompressed();


            auto& im = output.second;
            im.setZero();
            im.insert(5, 0) = std::complex(0.0, -1.0); // (2,1) -> 1*3+2=5 (col major!)
            im.insert(7, 0) = std::complex(0.0, 1.0); // (1,2) -> 2*3+1=7 (col major!)
            im.makeCompressed();

            return output;
        }


        std::pair<std::vector<dense_complex_elem_t>, std::vector<dense_complex_elem_t>> reference_dense_complex() {
            std::pair<std::vector<dense_complex_elem_t>, std::vector<dense_complex_elem_t>> output
                    = std::make_pair(std::vector<dense_complex_elem_t>(6, dense_complex_elem_t::Zero(2, 2)),
                                     std::vector<dense_complex_elem_t>(1, dense_complex_elem_t::Zero(2, 2)));
            auto& real = output.first;
            auto& im = output.second;

            real[0](0, 0) = 1.0;

            real[1](1, 1) = 1.0;

            real[4](0, 1) = std::complex(1.0, 1.0);
            real[4](1, 0) = std::complex(1.0, -1.0);

            im[0](0, 1) = std::complex(-1.0, 1.0);
            im[0](1, 0) = std::complex(-1.0, -1.0);

            return output;
        }

        std::pair<dense_complex_elem_t, dense_complex_elem_t> reference_dense_monolithic_complex() {
            std:std::pair<dense_complex_elem_t, dense_complex_elem_t> output
                = std::make_pair(dense_complex_elem_t::Zero(4, 6), dense_complex_elem_t::Zero(4, 1));

            auto& real = output.first;
            auto& im = output.second;

            real(0, 0) = 1.0;

            real(3, 1) = 1.0; // a

            real(1, 4) = std::complex(1.0, -1.0); // 4*=ab*
            real(2, 4) = std::complex(1.0, 1.0); // 4= ab

            im(1, 0) = std::complex(-1.0, -1.0);
            im(2, 0) = std::complex(-1.0, 1.0);

            return output;
        }
//
        std::pair<std::vector<sparse_complex_elem_t>, std::vector<sparse_complex_elem_t>> reference_sparse_complex() {
            std::pair<std::vector<sparse_complex_elem_t>, std::vector<sparse_complex_elem_t>> output;

            auto& real = output.first;
            auto& im = output.second;

            auto [dense_re, dense_im] = reference_dense_complex();
            for (const auto& b : dense_re) {
                real.emplace_back(b.sparseView());
            }
            for (const auto& b : dense_im) {
                im.emplace_back(b.sparseView());
            }

            return output;
        }

        std::pair<sparse_complex_elem_t, sparse_complex_elem_t> reference_sparse_monolithic_complex() {
            std::pair<sparse_complex_elem_t , sparse_complex_elem_t> output
                    = std::make_pair(sparse_complex_elem_t(4,6), sparse_complex_elem_t(4,1));

            auto& real = output.first;
            real.setZero();
            real.insert(0, 0) = 1.0;
            real.insert(3, 1) = 1.0;
            real.insert(1, 4) = std::complex(1.0, -1.0);
            real.insert(2, 4) = std::complex(1.0, 1.0);
            real.makeCompressed();


            auto& im = output.second;
            im.setZero();
            im.insert(1, 0) = std::complex(-1.0, -1.0);
            im.insert(2, 0) = std::complex(-1.0, 1.0);
            im.makeCompressed();

            return output;
        }

    }

    TEST(Operators_Matrix, DenseBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();

        const auto [id, mm] = ams.create_moment_matrix(1);
        ASSERT_EQ(symbol.size(), 7);

        const auto& [real, imaginary] = mm.Basis.Dense();
        const auto [ref_real, ref_imaginary] = reference_dense();

        assert_same_basis("Real", real, ref_real);
        assert_same_basis("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, DenseMonolithicBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();

        const auto [id, mm] = ams.create_moment_matrix(1);
        ASSERT_EQ(symbol.size(), 7);

        const auto& [real, imaginary] = mm.Basis.DenseMonolithic();
        const auto [ref_real, ref_imaginary] = reference_dense_monolithic();

        assert_same_matrix("Real", real, ref_real);
        assert_same_matrix("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, SparseBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();

        const auto [id, mm] = ams.create_moment_matrix(1);
        ASSERT_EQ(symbol.size(), 7);

        const auto& [real, imaginary] = mm.Basis.Sparse();
        const auto [ref_real, ref_imaginary] = reference_sparse();

        assert_same_basis("Real", real, ref_real);
        assert_same_basis("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, SparseMonolithicBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();

        const auto [id, mm] = ams.create_moment_matrix(1);
        ASSERT_EQ(symbol.size(), 7);

        const auto& [real, imaginary] = mm.Basis.SparseMonolithic();
        const auto [ref_real, ref_imaginary] = reference_sparse_monolithic();

        assert_same_matrix("Real", real, ref_real);
        assert_same_matrix("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, Level0LocalizingMatrixBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const SymbolTable& symbol = ams.Symbols();
        const auto [mm_id, mm] = ams.create_moment_matrix(1);
        ASSERT_EQ(symbol.size(), 7);
        ASSERT_EQ(symbol.Basis.RealSymbolCount(), 6);
        ASSERT_EQ(symbol.Basis.ImaginarySymbolCount(), 1);

        const auto [lmA_id, lmA_0] = ams.create_localizing_matrix(
                                LocalizingMatrixIndex(0, OperatorSequence{{0}, context}));
        ASSERT_EQ(symbol.size(), 7);
        ASSERT_EQ(symbol.Basis.RealSymbolCount(), 6);
        ASSERT_EQ(symbol.Basis.ImaginarySymbolCount(), 1);

        // Check sparse cell basis
        const auto& [real_cell, imaginary_cell] = lmA_0.Basis.Sparse();
        ASSERT_EQ(real_cell.size(), 6);
        ASSERT_EQ(imaginary_cell.size(), 1);


        // Check sparse basis
        ASSERT_NE(mm_id, lmA_id);
        const auto& [real, imaginary] = lmA_0.Basis.SparseMonolithic();
        ASSERT_EQ(real.cols(), 6);
        ASSERT_EQ(real.rows(), 1);
        EXPECT_EQ(real.nonZeros(), 1);
        EXPECT_EQ(real.coeff(0, 1), 1.0);

        ASSERT_EQ(imaginary.cols(), 1);
        ASSERT_EQ(imaginary.rows(), 1);
        EXPECT_EQ(imaginary.nonZeros(), 0);
    }


    TEST(Operators_Matrix, DenseComplexBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Brew our own matrix
        std::vector<SymbolExpression> matrix_data{
            SymbolExpression(1, 1.0),
            SymbolExpression(5, {1.0, 1.0}),
            SymbolExpression(5, {1.0, -1.0}, true),
            SymbolExpression(2, 1.0),
        };


        MonomialMatrix matrix{ams.Symbols(), ams.Context(),
                                std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        ASSERT_FALSE(matrix.real_coefficients());

        EXPECT_THROW([[maybe_unused]] const auto& bad = matrix.Basis.Dense(), Moment::errors::bad_basis_error);

        const auto& [real, imaginary] = matrix.Basis.DenseComplex();
        const auto [ref_real, ref_imaginary] = reference_dense_complex();

        assert_same_basis("Real", real, ref_real);
        assert_same_basis("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, DenseMonolithicComplexBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Brew our own matrix
        std::vector<SymbolExpression> matrix_data{
            SymbolExpression(1, 1.0),
            SymbolExpression(5, {1.0, 1.0}),
            SymbolExpression(5, {1.0, -1.0}, true),
            SymbolExpression(2, 1.0),
        };


        MonomialMatrix matrix{ams.Symbols(), ams.Context(),
                                std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        ASSERT_FALSE(matrix.real_coefficients());

        EXPECT_THROW([[maybe_unused]] const auto& bad = matrix.Basis.DenseMonolithic(), Moment::errors::bad_basis_error);

        const auto& [real, imaginary] = matrix.Basis.DenseMonolithicComplex();
        const auto [ref_real, ref_imaginary] = reference_dense_monolithic_complex();

        assert_same_matrix("Real", real, ref_real);
        assert_same_matrix("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, SparseComplexBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Brew our own matrix
        std::vector<SymbolExpression> matrix_data{
            SymbolExpression(1, 1.0),
            SymbolExpression(5, {1.0, 1.0}),
            SymbolExpression(5, {1.0, -1.0}, true),
            SymbolExpression(2, 1.0),
        };


        MonomialMatrix matrix{ams.Symbols(), ams.Context(),
                                std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        ASSERT_FALSE(matrix.real_coefficients());

        EXPECT_THROW([[maybe_unused]] const auto& bad = matrix.Basis.Sparse(), Moment::errors::bad_basis_error);

        const auto& [real, imaginary] = matrix.Basis.SparseComplex();
        const auto [ref_real, ref_imaginary] = reference_sparse_complex();

        assert_same_basis("Real", real, ref_real);
        assert_same_basis("Imaginary", imaginary, ref_imaginary);
    }

    TEST(Operators_Matrix, SparseMonolithicComplexBasis) {
        using namespace Moment::Algebraic;
        AlgebraicMatrixSystem ams{std::make_unique<AlgebraicContext>(2)};
        const SymbolTable& symbol = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        // Brew our own matrix
        std::vector<SymbolExpression> matrix_data{
            SymbolExpression(1, 1.0),
            SymbolExpression(5, {1.0, 1.0}),
            SymbolExpression(5, {1.0, -1.0}, true),
            SymbolExpression(2, 1.0),
        };


        MonomialMatrix matrix{ams.Symbols(), ams.Context(),
                                std::make_unique<SquareMatrix<SymbolExpression>>(2, std::move(matrix_data)), true};

        ASSERT_FALSE(matrix.real_coefficients());

        EXPECT_THROW([[maybe_unused]] const auto& bad = matrix.Basis.SparseMonolithic(), Moment::errors::bad_basis_error);

        const auto& [real, imaginary] = matrix.Basis.SparseMonolithicComplex();
        const auto [ref_real, ref_imaginary] = reference_sparse_monolithic_complex();

        assert_same_matrix("Real", real, ref_real);
        assert_same_matrix("Imaginary", imaginary, ref_imaginary);
    }


}