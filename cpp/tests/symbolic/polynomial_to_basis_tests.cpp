/**
 * polynomial_to_basis_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_to_basis.h"
#include "symbolic/symbol_table.h"

#include "scenarios/imported/imported_matrix_system.h"

#include "../scenarios/sparse_utils.h"

namespace Moment::Tests {
    
    class Symbolic_PolynomialToBasis : public ::testing::Test {
    private:
        std::unique_ptr<MatrixSystem> ms_ptr;

    protected:
        void SetUp() override {
            // One party, two symbols
            this->ms_ptr = std::make_unique<MatrixSystem>(std::make_unique<Context>(2));
            [[maybe_unused]] auto [id0, matLevel0] = ms_ptr->create_moment_matrix(0); // 0 1
            [[maybe_unused]] auto [id1, matLevel1] =  ms_ptr->create_moment_matrix(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1

            const auto& symbols = this->ms_ptr->Symbols();
            ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
            ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6);
            ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1);
            ASSERT_FALSE(symbols[5].is_hermitian());
        }

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ms_ptr->Symbols(); };

        static void compare_sparse_vectors(const Eigen::SparseVector<double>& actual, const Eigen::SparseVector<double>& expected) {
            ASSERT_EQ(actual.size(), expected.size()) << actual;
            ASSERT_EQ(actual.nonZeros(), expected.nonZeros()) << actual;
            for (auto iter = Eigen::SparseVector<double>::InnerIterator(actual); iter; ++iter) {
                EXPECT_EQ(iter.value(), expected.coeff(iter.index())) << "Index = " << iter.index();
            }
        }

        static void compare_sparse_vectors(const Eigen::SparseVector<std::complex<double>>& actual,
                                    const Eigen::SparseVector<std::complex<double>>& expected) {
            ASSERT_EQ(actual.size(), expected.size()) << actual;
            ASSERT_EQ(actual.nonZeros(), expected.nonZeros()) << actual;
            for (auto iter = Eigen::SparseVector<std::complex<double>>::InnerIterator(actual); iter; ++iter) {
                EXPECT_EQ(iter.value(), expected.coeff(iter.index())) << "Index = " << iter.index();
            }
        }
    };


    TEST_F(Symbolic_PolynomialToBasis, BasisToCombo_Scalars) {
        BasisVecToPolynomial convertor{this->get_symbols()};
        Polynomial scalar_one = convertor(make_sparse_vector({1.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                                          make_sparse_vector({0.0}));
        EXPECT_EQ(scalar_one, Polynomial::Scalar(1.0));

        Polynomial scalar_five = convertor(make_sparse_vector({5.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                                           make_sparse_vector({0.0}));
        EXPECT_EQ(scalar_five, Polynomial::Scalar(5.0));
    }

    TEST_F(Symbolic_PolynomialToBasis, BasisToCombo_Monomials) {
        BasisVecToPolynomial convertor{this->get_symbols()};

        Polynomial combo_a0 = convertor(make_sparse_vector({0.0, 1.0, 0.0, 0.0, 0.0, 0.0}),
                                        make_sparse_vector({0.0}));
        EXPECT_EQ(combo_a0, Polynomial({Monomial{2, 1.0}}));

        Polynomial combo_a1 = convertor(make_sparse_vector({0.0, 0.0, 1.0, 0.0, 0.0, 0.0}),
                                        make_sparse_vector({0.0}));
        EXPECT_EQ(combo_a1, Polynomial({Monomial{3, 1.0}}));

        Polynomial combo_a0a0 = convertor(make_sparse_vector({0.0, 0.0, 0.0, 1.0, 0.0, 0.0}),
                                          make_sparse_vector({0.0}));
        EXPECT_EQ(combo_a0a0, Polynomial({Monomial{4, 1.0}}));

        Polynomial combo_a1a1 = convertor(make_sparse_vector({0.0, 0.0, 0.0, 0.0, 0.0, 1.0}),
                                          make_sparse_vector({0.0}));
        EXPECT_EQ(combo_a1a1, Polynomial({Monomial{6, 1.0}}));

        // Non-trivial element a0a1 has support in real and imaginary parts of basis:
        Polynomial combo_a0a1 = convertor(make_sparse_vector({0.0, 0.0, 0.0, 0.0, 1.0, 0.0}),
                                          make_sparse_vector({1.0}));
        EXPECT_EQ(combo_a0a1, Polynomial({Monomial{5, 1.0}}));
    }

    TEST_F(Symbolic_PolynomialToBasis, BasisToCombo_OutOfBounds) {
        // One party, two symbols
        BasisVecToPolynomial convertor{this->get_symbols()};

        EXPECT_THROW([[maybe_unused]] auto x = convertor(
                             make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0.0, 1.0}), make_sparse_vector<double>({0.0})),
                     Moment::errors::unknown_basis_elem);
        EXPECT_THROW([[maybe_unused]] auto x = convertor(
                             make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0.0}), make_sparse_vector<double>({0.0, 1.0})),
                     Moment::errors::unknown_basis_elem);
    }

    TEST_F(Symbolic_PolynomialToBasis, BasisToCombo_HermAntiHermTerms) {
        BasisVecToPolynomial convertor{this->get_symbols()};
        Polynomial combo_a0a1_hermitian = convertor(make_sparse_vector({0.0, 0.0, 0.0, 0.0, 1.0, 0.0}),
                                                    make_sparse_vector({0.0}));
        EXPECT_EQ(combo_a0a1_hermitian, Polynomial({Monomial{5, 0.5, false},
                                                    Monomial{5, 0.5, true}}));

        Polynomial combo_a0a1_antihermitian = convertor(
                make_sparse_vector({0.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                make_sparse_vector({1.0}));
        EXPECT_EQ(combo_a0a1_antihermitian, Polynomial({Monomial{5, 0.5, false},
                                                        Monomial{5, -0.5, true}}));
    }

    TEST_F(Symbolic_PolynomialToBasis, ComplexBasisToCombo_Scalars) {
        ComplexBasisVecToPolynomial convertor{this->get_symbols()};

        Polynomial scalar_one = convertor(make_sparse_vector({1.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                                          make_sparse_vector({0.0}));
        EXPECT_EQ(scalar_one, Polynomial::Scalar(1.0));

        Polynomial scalar_five = convertor(make_sparse_vector({5.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                                           make_sparse_vector({0.0}));
        EXPECT_EQ(scalar_five, Polynomial::Scalar(5.0));

        Polynomial scalar_five_plus_two_i = convertor(make_sparse_vector<std::complex<double>>({{5.0, 2.0}, 0, 0, 0, 0, 0}),
                                                      make_sparse_vector<std::complex<double>>({0}));
        EXPECT_EQ(scalar_five_plus_two_i, Polynomial::Scalar(std::complex<double>(5.0, 2.0)));

    }

    TEST_F(Symbolic_PolynomialToBasis, ComplexBasisToCombo_Monomials) {
        ComplexBasisVecToPolynomial convertor{this->get_symbols()};

        Polynomial combo_a0 = convertor(make_sparse_vector<std::complex<double>>({0, 1.0, 0, 0, 0, 0}),
                                        make_sparse_vector<std::complex<double>>({0}));
        EXPECT_EQ(combo_a0, Polynomial({Monomial{2, 1.0}}));

        Polynomial combo_a1 = convertor(make_sparse_vector<std::complex<double>>({0, 0, 1.0, 0, 0, 0}),
                                        make_sparse_vector<std::complex<double>>({0}));
        EXPECT_EQ(combo_a1, Polynomial({Monomial{3, 1.0}}));

        Polynomial combo_a0a0 = convertor(make_sparse_vector<std::complex<double>>({0, 0, 0, {1.0, 2.0}, 0, 0}),
                                          make_sparse_vector<std::complex<double>>({0}));
        EXPECT_EQ(combo_a0a0, Polynomial({Monomial{4, {1.0, 2.0}}}));

        Polynomial combo_a1a1 = convertor(make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 0, 1.0}),
                                          make_sparse_vector<std::complex<double>>({0}));
        EXPECT_EQ(combo_a1a1, Polynomial({Monomial{6, 1.0}}));

        // Non-trivial element a0a1 has support in real and imaginary parts of basis:
        Polynomial combo_a0a1 = convertor(make_sparse_vector<std::complex<double>>({0, 0, 0, 0, {2.0, 1.0}, 0.0}),
                                          make_sparse_vector<std::complex<double>>({{2.0, 1.0}}));
        EXPECT_EQ(combo_a0a1, Polynomial({Monomial{5, {2.0, 1.0}}}));
    }

    TEST_F(Symbolic_PolynomialToBasis, ComplexBasisToCombo_HermAntiHermTerms) {
        ComplexBasisVecToPolynomial convertor{this->get_symbols()};

        Polynomial combo_a0a1_hermitian = convertor(make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0.0}),
                                                    make_sparse_vector<std::complex<double>>({0.0}));
        EXPECT_EQ(combo_a0a1_hermitian, Polynomial({Monomial{5, 0.5, false},
                                                    Monomial{5, 0.5, true}}));

        Polynomial combo_a0a1_antihermitian = convertor(
                make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 0.0, 0.0}),
                make_sparse_vector<std::complex<double>>({1.0}));
        EXPECT_EQ(combo_a0a1_antihermitian, Polynomial({Monomial{5, 0.5, false},
                                                        Monomial{5, -0.5, true}}));
    }
    
    TEST_F(Symbolic_PolynomialToBasis, ComboToBasis_Scalars) {
        PolynomialToBasisVec convertor{this->get_symbols()};

        auto [scalar_one_re, scalar_one_im] = convertor(Polynomial::Scalar(1.0));
        compare_sparse_vectors(scalar_one_re, make_sparse_vector<double>({1.0, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_one_im, make_sparse_vector<double>({0}));

        auto [scalar_five_re, scalar_five_im] = convertor(Polynomial::Scalar(5.0));
        compare_sparse_vectors(scalar_five_re, make_sparse_vector<double>({5.0, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_five_im, make_sparse_vector<double>({0}));
    }

    TEST_F(Symbolic_PolynomialToBasis, ComboToBasis_Monomials) {
        PolynomialToBasisVec convertor{this->get_symbols()};

        auto [a0_re, a0_im] = convertor(Polynomial{{Monomial(2, 1.0)}});
        compare_sparse_vectors(a0_re, make_sparse_vector<double>({0, 1.0, 0, 0, 0, 0}));
        compare_sparse_vectors(a0_im, make_sparse_vector<double>({0}));

        auto [a1_re, a1_im] = convertor(Polynomial{{Monomial(3, 1.0)}});
        compare_sparse_vectors(a1_re, make_sparse_vector<double>({0, 0, 1.0, 0, 0, 0}));
        compare_sparse_vectors(a1_im, make_sparse_vector<double>({0}));

        auto [a0a0_re, a0a0_im] = convertor(Polynomial{{Monomial(4, 1.0)}});
        compare_sparse_vectors(a0a0_re, make_sparse_vector<double>({0, 0, 0, 1.0, 0, 0}));
        compare_sparse_vectors(a0a0_im, make_sparse_vector<double>({0}));

        auto [a1a1_re, a1a1_im] = convertor(Polynomial{{Monomial(6, 1.0)}});
        compare_sparse_vectors(a1a1_re, make_sparse_vector<double>({0, 0, 0, 0, 0, 1.0}));
        compare_sparse_vectors(a1a1_im, make_sparse_vector<double>({0}));

        auto [a0a1_re, a0a1_im] = convertor(Polynomial{{Monomial(5, 1.0)}});
        compare_sparse_vectors(a0a1_re, make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_im, make_sparse_vector<double>({1.0}));

        auto [a0a1_star_re, a0a1_star_im] = convertor(Polynomial{{Monomial(5, 1.0, true)}});
        compare_sparse_vectors(a0a1_star_re, make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_star_im, make_sparse_vector<double>({-1.0}));
    }

    TEST_F(Symbolic_PolynomialToBasis, ComboToBasis_HermAntiHerm) {
        PolynomialToBasisVec convertor{this->get_symbols()};

        auto [a0a1_a1a0_re, a0a1_a1a0_im] = convertor(Polynomial{{Monomial(5, 0.5, false),
                                                                  Monomial(5, 0.5, true)}});
        compare_sparse_vectors(a0a1_a1a0_re, make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_a1a0_im, make_sparse_vector<double>({0.0}));

        auto [a0a1_minus_a1a0_re, a0a1_minus_a1a0_im] = convertor(
                Polynomial{{Monomial(5, 0.5, false),
                            Monomial(5, -0.5, true)}});
        compare_sparse_vectors(a0a1_minus_a1a0_re, make_sparse_vector<double>({0, 0, 0, 0, 0.0, 0}));
        compare_sparse_vectors(a0a1_minus_a1a0_im, make_sparse_vector<double>({1.0}));
    }
    TEST_F(Symbolic_PolynomialToBasis, ComboToComplexBasis_Scalars) {
        PolynomialToComplexBasisVec convertor{this->get_symbols()};

        auto [scalar_one_re, scalar_one_im] = convertor(Polynomial::Scalar(1.0));
        compare_sparse_vectors(scalar_one_re, make_sparse_vector<std::complex<double>>({1.0, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_one_im, make_sparse_vector<std::complex<double>>({0}));

        auto [scalar_five_re, scalar_five_im] = convertor(Polynomial::Scalar(5.0));
        compare_sparse_vectors(scalar_five_re, make_sparse_vector<std::complex<double>>({5.0, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_five_im, make_sparse_vector<std::complex<double>>({0}));

        auto [scalar_5_2i_re, scalar_5_2i_im] = convertor(Polynomial::Scalar({5.0, 2.0}));
        compare_sparse_vectors(scalar_5_2i_re, make_sparse_vector<std::complex<double>>({{5.0, 2.0}, 0, 0, 0, 0, 0}));
        compare_sparse_vectors(scalar_5_2i_im, make_sparse_vector<std::complex<double>>({0}));
    }

    TEST_F(Symbolic_PolynomialToBasis, ComboToComplexBasis_Monomials) {
        PolynomialToComplexBasisVec convertor{this->get_symbols()};

        auto [a0_re, a0_im] = convertor(Polynomial{{Monomial(2, 1.0)}});
        compare_sparse_vectors(a0_re, make_sparse_vector<std::complex<double>>({0, 1.0, 0, 0, 0, 0}));
        compare_sparse_vectors(a0_im, make_sparse_vector<std::complex<double>>({0}));

        auto [a1_re, a1_im] = convertor(Polynomial{{Monomial(3, 1.0)}});
        compare_sparse_vectors(a1_re, make_sparse_vector<std::complex<double>>({0, 0, 1.0, 0, 0, 0}));
        compare_sparse_vectors(a1_im, make_sparse_vector<std::complex<double>>({0}));

        auto [a0a0_re, a0a0_im] = convertor(Polynomial{{Monomial(4, 1.0)}});
        compare_sparse_vectors(a0a0_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 1.0, 0, 0}));
        compare_sparse_vectors(a0a0_im, make_sparse_vector<std::complex<double>>({0}));

        auto [a1a1_re, a1a1_im] = convertor(Polynomial{{Monomial(6, 1.0)}});
        compare_sparse_vectors(a1a1_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 0, 1.0}));
        compare_sparse_vectors(a1a1_im, make_sparse_vector<std::complex<double>>({0}));

        auto [a0a1_re, a0a1_im] = convertor(Polynomial{{Monomial(5, 1.0)}});
        compare_sparse_vectors(a0a1_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_im, make_sparse_vector<std::complex<double>>({1.0}));

        auto [a0a1_star_re, a0a1_star_im] = convertor(Polynomial{{Monomial(5, 1.0, true)}});
        compare_sparse_vectors(a0a1_star_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_star_im, make_sparse_vector<std::complex<double>>({-1.0}));
    }

    TEST_F(Symbolic_PolynomialToBasis, ComboToComplexBasis_HermAntiHerm) {
        PolynomialToComplexBasisVec convertor{this->get_symbols()};

        auto [a0a1_a1a0_re, a0a1_a1a0_im] = convertor(Polynomial{{Monomial(5, 0.5, false),
                                                                  Monomial(5, 0.5, true)}});
        compare_sparse_vectors(a0a1_a1a0_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_a1a0_im, make_sparse_vector<std::complex<double>>({0.0}));

        auto [a0a1_minus_a1a0_re, a0a1_minus_a1a0_im] = convertor(
                Polynomial{{Monomial(5, 0.5, false),
                            Monomial(5, -0.5, true)}});
        compare_sparse_vectors(a0a1_minus_a1a0_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 0.0, 0}));
        compare_sparse_vectors(a0a1_minus_a1a0_im, make_sparse_vector<std::complex<double>>({1.0}));
    }

}