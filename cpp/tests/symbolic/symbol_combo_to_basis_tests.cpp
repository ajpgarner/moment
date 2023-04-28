/**
 * symbol_combo_to_basis_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symbolic/symbol_combo.h"
#include "symbolic/symbol_combo_to_basis.h"
#include "symbolic/symbol_table.h"

#include "scenarios/imported/imported_matrix_system.h"

#include "../scenarios/sparse_utils.h"

namespace Moment::Tests {

    void compare_sparse_vectors(const Eigen::SparseVector<double>& actual, const Eigen::SparseVector<double>& expected) {
        ASSERT_EQ(actual.size(), expected.size()) << actual;
        ASSERT_EQ(actual.nonZeros(), expected.nonZeros()) << actual;
        for (auto iter = Eigen::SparseVector<double>::InnerIterator(actual); iter; ++iter) {
            EXPECT_EQ(iter.value(), expected.coeff(iter.index())) << "Index = " << iter.index();
        }
    }

    TEST(Symbolic_SymbolComboToBasis, ComboFromBasis) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto &symbols = system.Symbols();

        [[maybe_unused]] auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        [[maybe_unused]] auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
        ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6);
        ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1);
        ASSERT_FALSE(symbols[5].is_hermitian());

        SymbolCombo scalar_one = BasisVecToSymbolCombo{symbols}(make_sparse_vector({1.0, 0, 0, 0, 0, 0}),
                                                                make_sparse_vector({0}));
        EXPECT_EQ(scalar_one, SymbolCombo::Scalar(1.0));

        SymbolCombo scalar_five = BasisVecToSymbolCombo{symbols}(make_sparse_vector({5.0, 0, 0, 0, 0, 0}),
                                                                 make_sparse_vector({0}));
        EXPECT_EQ(scalar_five, SymbolCombo::Scalar(5.0));

        SymbolCombo combo_a0 = BasisVecToSymbolCombo{symbols}(make_sparse_vector({0, 1.0, 0, 0, 0, 0}),
                                                              make_sparse_vector({0}));
        EXPECT_EQ(combo_a0, SymbolCombo({SymbolExpression{2, 1.0}}));

        SymbolCombo combo_a1 = BasisVecToSymbolCombo{symbols}(make_sparse_vector({0, 0, 1.0, 0, 0, 0}),
                                                              make_sparse_vector({0}));
        EXPECT_EQ(combo_a1, SymbolCombo({SymbolExpression{3, 1.0}}));

        SymbolCombo combo_a0a0 = BasisVecToSymbolCombo{symbols}(make_sparse_vector({0, 0, 0, 1.0, 0, 0}),
                                                                make_sparse_vector({0}));
        EXPECT_EQ(combo_a0a0, SymbolCombo({SymbolExpression{4, 1.0}}));

        SymbolCombo combo_a1a1 = BasisVecToSymbolCombo{symbols}(make_sparse_vector({0, 0, 0, 0, 0, 1.0}),
                                                                make_sparse_vector({0}));
        EXPECT_EQ(combo_a1a1, SymbolCombo({SymbolExpression{6, 1.0}}));

        // Non-trivial element a0a1 has support in real and imaginary parts of basis:

        SymbolCombo combo_a0a1 = BasisVecToSymbolCombo{symbols}(make_sparse_vector({0, 0, 0, 0, 1.0, 0.0}),
                                                                make_sparse_vector({1.0}));
        EXPECT_EQ(combo_a0a1, SymbolCombo({SymbolExpression{5, 1.0}}));

        SymbolCombo combo_a0a1_hermitian = BasisVecToSymbolCombo{symbols}(make_sparse_vector({0, 0, 0, 0, 1.0, 0.0}),
                                                                          make_sparse_vector({0.0}));
        EXPECT_EQ(combo_a0a1_hermitian, SymbolCombo({SymbolExpression{5, 0.5, false},
                                                     SymbolExpression{5, 0.5, true}}));

        SymbolCombo combo_a0a1_antihermitian = BasisVecToSymbolCombo{symbols}(
                make_sparse_vector({0, 0, 0, 0, 0.0, 0.0}),
                make_sparse_vector({1.0}));
        EXPECT_EQ(combo_a0a1_antihermitian, SymbolCombo({SymbolExpression{5, 0.5, false},
                                                         SymbolExpression{5, -0.5, true}}));

    }

    TEST(Symbolic_SymbolComboToBasis, ComboFromBasis_OutOfBounds) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto &symbols = system.Symbols();

        [[maybe_unused]] auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        [[maybe_unused]] auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
        ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6);
        ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1);


        EXPECT_THROW([[maybe_unused]] auto x = BasisVecToSymbolCombo{symbols}(
                            make_sparse_vector({0, 0, 0, 0, 1.0, 0.0, 1.0}), make_sparse_vector({0.0})),
                     Moment::errors::unknown_basis_elem);
        EXPECT_THROW([[maybe_unused]] auto x = BasisVecToSymbolCombo{symbols}(
                            make_sparse_vector({0, 0, 0, 0, 1.0, 0.0}), make_sparse_vector({0.0, 1.0})),
                     Moment::errors::unknown_basis_elem);

    }


    TEST(Symbolic_SymbolComboToBasis, BasisFromCombo) {
        // One party, two symbols
        MatrixSystem system{std::make_unique<Context>(2)};
        const auto &symbols = system.Symbols();

        [[maybe_unused]] auto [id0, matLevel0] = system.create_moment_matrix(0); // 0 1
        [[maybe_unused]] auto [id1, matLevel1] = system.create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1
        ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
        ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6);
        ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1);
        ASSERT_FALSE(symbols[5].is_hermitian());

        auto [scalar_one_re, scalar_one_im] = SymbolComboToBasisVec{symbols}(SymbolCombo::Scalar(1.0));
        compare_sparse_vectors(scalar_one_re, make_sparse_vector({1.0, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_one_im, make_sparse_vector({0}));

        auto [scalar_five_re, scalar_five_im] = SymbolComboToBasisVec{symbols}(SymbolCombo::Scalar(5.0));
        compare_sparse_vectors(scalar_five_re, make_sparse_vector({5.0, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_five_im, make_sparse_vector({0}));

        auto [a0_re, a0_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(2, 1.0)}});
        compare_sparse_vectors(a0_re, make_sparse_vector({0, 1.0, 0, 0, 0, 0}));
        compare_sparse_vectors(a0_im, make_sparse_vector({0}));

        auto [a1_re, a1_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(3, 1.0)}});
        compare_sparse_vectors(a1_re, make_sparse_vector({0, 0, 1.0, 0, 0, 0}));
        compare_sparse_vectors(a1_im, make_sparse_vector({0}));

        auto [a0a0_re, a0a0_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(4, 1.0)}});
        compare_sparse_vectors(a0a0_re, make_sparse_vector({0, 0, 0, 1.0, 0, 0}));
        compare_sparse_vectors(a0a0_im, make_sparse_vector({0}));

        auto [a1a1_re, a1a1_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(6, 1.0)}});
        compare_sparse_vectors(a1a1_re, make_sparse_vector({0, 0, 0, 0, 0, 1.0}));
        compare_sparse_vectors(a1a1_im, make_sparse_vector({0}));

        auto [a0a1_re, a0a1_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(5, 1.0)}});
        compare_sparse_vectors(a0a1_re, make_sparse_vector({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_im, make_sparse_vector({1.0}));

        auto [a0a1_star_re, a0a1_star_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(5, 1.0, true)}});
        compare_sparse_vectors(a0a1_star_re, make_sparse_vector({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_star_im, make_sparse_vector({-1.0}));


        auto [a0a1_a1a0_re, a0a1_a1a0_im] = SymbolComboToBasisVec{symbols}(SymbolCombo{{SymbolExpression(5, 0.5, false),
                                                                                        SymbolExpression(5, 0.5, true)}});
        compare_sparse_vectors(a0a1_a1a0_re, make_sparse_vector({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_a1a0_im, make_sparse_vector({0.0}));

        auto [a0a1_minus_a1a0_re, a0a1_minus_a1a0_im] = SymbolComboToBasisVec{symbols}(
                                                            SymbolCombo{{SymbolExpression(5, 0.5, false),
                                                                         SymbolExpression(5, -0.5, true)}});
        compare_sparse_vectors(a0a1_minus_a1a0_re, make_sparse_vector({0, 0, 0, 0, 0.0, 0}));
        compare_sparse_vectors(a0a1_minus_a1a0_im, make_sparse_vector({1.0}));
    }
}