/**
 * polynomial_to_basis_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "symbolic/polynomial.h"
#include "symbolic/polynomial_factory.h"
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
            [[maybe_unused]] auto [id0, matLevel0] = ms_ptr->MomentMatrix.create(0); // 0 1
            [[maybe_unused]] auto [id1, matLevel1] =  ms_ptr->MomentMatrix.create(1); // 0 1 a0 a1 a0a0 a0a1 (a1a0=a0a1*) a1a1

            const auto& symbols = this->ms_ptr->Symbols();
            ASSERT_EQ(symbols.size(), 7); // 0 1 a0 a1 a0a0 a0a1(=a1a0*) a1a1
            ASSERT_EQ(symbols.Basis.RealSymbolCount(), 6);
            ASSERT_EQ(symbols.Basis.ImaginarySymbolCount(), 1);
            ASSERT_FALSE(symbols[5].is_hermitian());
        }

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ms_ptr->Symbols(); }

        [[nodiscard]] const PolynomialFactory& get_factory() noexcept { return this->ms_ptr->polynomial_factory(); }

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

        template<typename scalar_t>
        static void compare_sparse_zero(const Eigen::SparseVector<scalar_t>& actual, Eigen::Index size) {
            ASSERT_EQ(actual.size(), size) << actual;
            ASSERT_EQ(actual.nonZeros(), 0) << actual;
        }

    };


    TEST_F(Symbolic_PolynomialToBasis, BasisToPolynomial_Scalars) {
        BasisVecToPolynomial convertor{this->get_factory()};
        Polynomial scalar_one = convertor(make_sparse_vector({1.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                                          make_sparse_vector({0.0}));
        EXPECT_EQ(scalar_one, Polynomial::Scalar(1.0));

        Polynomial scalar_five = convertor(make_sparse_vector({5.0, 0.0, 0.0, 0.0, 0.0, 0.0}),
                                           make_sparse_vector({0.0}));
        EXPECT_EQ(scalar_five, Polynomial::Scalar(5.0));
    }

    TEST_F(Symbolic_PolynomialToBasis, BasisToPolynomial_Monomials) {
        BasisVecToPolynomial convertor{this->get_factory()};

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

    TEST_F(Symbolic_PolynomialToBasis, BasisToPolynomial_OutOfBounds) {
        // One party, two symbols
        BasisVecToPolynomial convertor{this->get_factory()};

        EXPECT_THROW([[maybe_unused]] auto x = convertor(
                             make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0.0, 1.0}), make_sparse_vector<double>({0.0})),
                     Moment::errors::unknown_basis_elem);
        EXPECT_THROW([[maybe_unused]] auto x = convertor(
                             make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0.0}), make_sparse_vector<double>({0.0, 1.0})),
                     Moment::errors::unknown_basis_elem);
    }

    TEST_F(Symbolic_PolynomialToBasis, BasisToPolynomial_HermAntiHermTerms) {
        BasisVecToPolynomial convertor{this->get_factory()};
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

    TEST_F(Symbolic_PolynomialToBasis, ComplexBasisToPolynomial_Scalars) {
        ComplexBasisVecToPolynomial convertor{this->get_factory()};

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

    TEST_F(Symbolic_PolynomialToBasis, ComplexBasisToPolynomial_Monomials) {
        ComplexBasisVecToPolynomial convertor{this->get_factory()};

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

    TEST_F(Symbolic_PolynomialToBasis, ComplexBasisToPolynomial_HermAntiHermTerms) {
        ComplexBasisVecToPolynomial convertor{this->get_factory()};

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
    
    TEST_F(Symbolic_PolynomialToBasis, PolynomialToBasis_Scalars) {
        PolynomialToBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

        auto [scalar_one_re, scalar_one_im] = convertor(Polynomial::Scalar(1.0));
        compare_sparse_vectors(scalar_one_re.real, make_sparse_vector<double>({1.0, 0, 0, 0, 0, 0}));
        compare_sparse_zero(scalar_one_re.imaginary, 1);
        compare_sparse_zero(scalar_one_im.real, 6);
        compare_sparse_zero(scalar_one_im.imaginary, 1);

        auto [scalar_five_re, scalar_five_im] = convertor(Polynomial::Scalar(5.0));
        compare_sparse_vectors(scalar_five_re.real, make_sparse_vector<double>({5.0, 0, 0, 0, 0, 0}));
        compare_sparse_zero(scalar_five_re.imaginary, 1);
        compare_sparse_zero(scalar_five_im.real, 6);
        compare_sparse_zero(scalar_five_im.imaginary, 1);
    }


    TEST_F(Symbolic_PolynomialToBasis, PolynomialToBasis_Monomials) {
        PolynomialToBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

        auto [a0_re, a0_im] = convertor(Polynomial{{Monomial(2, 1.0)}});
        compare_sparse_vectors(a0_re.real, make_sparse_vector<double>({0, 1.0, 0, 0, 0, 0}));
        compare_sparse_zero(a0_re.imaginary, 1);
        compare_sparse_zero(a0_im.real, 6);
        compare_sparse_zero(a0_im.imaginary, 1);

        auto [a1_re, a1_im] = convertor(Polynomial{{Monomial(3, 1.0)}});
        compare_sparse_vectors(a1_re.real, make_sparse_vector<double>({0, 0, 1.0, 0, 0, 0}));
        compare_sparse_zero(a1_re.imaginary, 1);
        compare_sparse_zero(a1_im.real, 6);
        compare_sparse_zero(a1_im.imaginary, 1);

        auto [a0a0_re, a0a0_im] = convertor(Polynomial{{Monomial(4, 1.0)}});
        compare_sparse_vectors(a0a0_re.real, make_sparse_vector<double>({0, 0, 0, 1.0, 0, 0}));
        compare_sparse_zero(a0a0_re.imaginary, 1);
        compare_sparse_zero(a0a0_im.real, 6);
        compare_sparse_zero(a0a0_im.imaginary, 1);

        auto [a1a1_re, a1a1_im] = convertor(Polynomial{{Monomial(6, 1.0)}});
        compare_sparse_vectors(a1a1_re.real, make_sparse_vector<double>({0, 0, 0, 0, 0, 1.0}));
        compare_sparse_zero(a1a1_re.imaginary, 1);
        compare_sparse_zero(a1a1_im.real, 6);
        compare_sparse_zero(a1a1_im.imaginary, 1);

        auto [a0a1_re, a0a1_im] = convertor(Polynomial{{Monomial(5, 1.0)}});
        compare_sparse_vectors(a0a1_re.real, make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_zero(a0a1_re.imaginary, 1);
        compare_sparse_zero(a0a1_im.real, 6);
        compare_sparse_vectors(a0a1_im.imaginary, make_sparse_vector<double>({1.0}));

        auto [a0a1_star_re, a0a1_star_im] = convertor(Polynomial{{Monomial(5, 1.0, true)}});
        compare_sparse_vectors(a0a1_star_re.real, make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_zero(a0a1_star_re.imaginary, 1);
        compare_sparse_zero(a0a1_star_im.real, 6);
        compare_sparse_vectors(a0a1_star_im.imaginary, make_sparse_vector<double>({-1.0}));

    }

    TEST_F(Symbolic_PolynomialToBasis, PolynomialToBasis_HermAntiHerm) {
        PolynomialToBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

        auto [a0a1_a1a0_re, a0a1_a1a0_im] = convertor(Polynomial{{Monomial(5, 0.5, false),
                                                                  Monomial(5, 0.5, true)}}); // Re(Z)
        compare_sparse_vectors(a0a1_a1a0_re.real, make_sparse_vector<double>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_zero(a0a1_a1a0_re.imaginary, 1);
        compare_sparse_zero(a0a1_a1a0_im.real, 6);
        compare_sparse_zero(a0a1_a1a0_im.imaginary, 1);

        auto [a0a1_minus_a1a0_re, a0a1_minus_a1a0_im] = convertor(
                Polynomial{{Monomial(5, std::complex{0.0, -0.5}, false),
                            Monomial(5, std::complex{0.0, 0.5}, true)}}); // Im (Z)
        compare_sparse_zero(a0a1_minus_a1a0_im.real, 6);
        compare_sparse_vectors(a0a1_minus_a1a0_re.imaginary, make_sparse_vector<double>({1.0}));
        compare_sparse_zero(a0a1_minus_a1a0_im.real, 6);
        compare_sparse_zero(a0a1_minus_a1a0_im.imaginary, 1);

        auto [i_im_Z_re, i_im_Z_im] = convertor(
                Polynomial{{Monomial(5, std::complex{0.5}, false),
                            Monomial(5, std::complex{-0.5}, true)}}); // Im (Z)
        compare_sparse_zero(i_im_Z_re.real, 6);
        compare_sparse_zero(i_im_Z_re.imaginary, 1);
        compare_sparse_zero(i_im_Z_im.real, 6);
        compare_sparse_vectors(i_im_Z_im.imaginary, make_sparse_vector<double>({1.0}));
    }


    TEST_F(Symbolic_PolynomialToBasis, PolynomialToComplexBasis_Scalars) {
        PolynomialToComplexBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

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

    TEST_F(Symbolic_PolynomialToBasis, PolynomialToComplexBasis_RealMonomials) {
        PolynomialToComplexBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

        auto [a0_re, a0_im] = convertor(Polynomial{{Monomial(2, 1.0)}});
        compare_sparse_vectors(a0_re, make_sparse_vector<std::complex<double>>({0, 1.0, 0, 0, 0, 0}));
        compare_sparse_zero(a0_im, 1);

        auto [a1_re, a1_im] = convertor(Polynomial{{Monomial(3, 1.0)}});
        compare_sparse_vectors(a1_re, make_sparse_vector<std::complex<double>>({0, 0, 1.0, 0, 0, 0}));
        compare_sparse_zero(a1_im, 1);

        auto [a0a0_re, a0a0_im] = convertor(Polynomial{{Monomial(4, 1.0)}});
        compare_sparse_vectors(a0a0_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 1.0, 0, 0}));
        compare_sparse_zero(a0a0_im, 1);

        auto [a1a1_re, a1a1_im] = convertor(Polynomial{{Monomial(6, 1.0)}});
        compare_sparse_vectors(a1a1_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 0, 1.0}));
        compare_sparse_zero(a1a1_im, 1);
    }

    TEST_F(Symbolic_PolynomialToBasis, PolynomialToComplexBasis_ComplexMonomials) {
        PolynomialToComplexBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

        auto [a0a1_re, a0a1_im] = convertor(Polynomial{{Monomial(5, 1.0)}});
        compare_sparse_vectors(a0a1_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_im, make_sparse_vector<std::complex<double>>({{0.0, 1.0}}));

        auto [a0a1_star_re, a0a1_star_im] = convertor(Polynomial{{Monomial(5, 1.0, true)}});
        compare_sparse_vectors(a0a1_star_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_vectors(a0a1_star_im, make_sparse_vector<std::complex<double>>({{0.0, -1.0}}));

        // i a0a1:
        auto [rot_re, rot_im] = convertor(Polynomial{{Monomial(5, std::complex{0.0, 1.0})}});
        compare_sparse_vectors(rot_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, {0.0, 1.0}, 0}));
        compare_sparse_vectors(rot_im, make_sparse_vector<std::complex<double>>({{-1.0, 0.0}}));

    }

    TEST_F(Symbolic_PolynomialToBasis, PolynomialToComplexBasis_HermAntiHerm) {
        PolynomialToComplexBasisVec convertor{this->get_symbols(), this->get_factory().zero_tolerance};

        auto [a0a1_a1a0_re, a0a1_a1a0_im] = convertor(Polynomial{{Monomial(5, 0.5, false),
                                                                  Monomial(5, 0.5, true)}});
        compare_sparse_vectors(a0a1_a1a0_re, make_sparse_vector<std::complex<double>>({0, 0, 0, 0, 1.0, 0}));
        compare_sparse_zero(a0a1_a1a0_im, 1);

        auto [a0a1_minus_a1a0_re, a0a1_minus_a1a0_im] = convertor(
                Polynomial{{Monomial(5, 0.5, false),
                            Monomial(5, -0.5, true)}});
        compare_sparse_zero(a0a1_minus_a1a0_re, 6);
        compare_sparse_vectors(a0a1_minus_a1a0_im, make_sparse_vector<std::complex<double>>({std::complex{0.0, 1.0}}));
    }

}