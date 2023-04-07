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

}