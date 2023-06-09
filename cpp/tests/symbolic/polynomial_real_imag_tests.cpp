/**
 * polynomial_real_imag_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "symbolic/polynomial.h"
#include "symbolic/symbol_table.h"

#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic_matrix_helpers.h"


namespace Moment::Tests {

    class Symbolic_Polynomial_ReIm : public ::testing::Test {
    private:
        std::unique_ptr<Algebraic::AlgebraicMatrixSystem> ams_ptr;
        std::unique_ptr<PolynomialFactory> factory_ptr;

    protected:
        symbol_name_t id_e, id_a, id_b, id_aa, id_ab, id_bb;

    protected:
        void SetUp() override {
            ams_ptr = std::make_unique<Algebraic::AlgebraicMatrixSystem>(
                    std::make_unique<Algebraic::AlgebraicContext>(2)
            );
            ams_ptr->generate_dictionary(2); // e, a, b, aa, ab (ba), bb
            factory_ptr = std::make_unique<ByIDPolynomialFactory>(ams_ptr->Symbols());

            const auto& symbols = this->ams_ptr->Symbols();
            const auto& context = this->ams_ptr->AlgebraicContext();

            this->id_e = find_or_fail(symbols, OperatorSequence::Identity(context));
            this->id_a = find_or_fail(symbols, OperatorSequence{{0}, context});
            this->id_b = find_or_fail(symbols, OperatorSequence{{1}, context});
            this->id_aa = find_or_fail(symbols, OperatorSequence{{0, 0}, context});
            this->id_ab = find_or_fail(symbols, OperatorSequence{{0, 1}, context});
            this->id_bb = find_or_fail(symbols, OperatorSequence{{1, 1}, context});
        }

        [[nodiscard]] Algebraic::AlgebraicMatrixSystem& get_system() const noexcept {
            return *this->ams_ptr;
        }

        [[nodiscard]] const Algebraic::AlgebraicContext& get_context() const noexcept {
            return this->ams_ptr->AlgebraicContext();
        };

        [[nodiscard]] SymbolTable& get_symbols() noexcept { return this->ams_ptr->Symbols(); };

        [[nodiscard]] const PolynomialFactory& get_factory() const noexcept { return *this->factory_ptr; };

        void expect_approximately_equal(const Polynomial& LHS, const Polynomial& RHS, double tolerance) {
            EXPECT_TRUE(LHS.approximately_equals(RHS, tolerance)) << "LHS = \n" << LHS << "\n RHS = \n" << RHS;
        }

    };


    TEST_F(Symbolic_Polynomial_ReIm, Real_Empty) {
        const auto& factory = this->get_factory();
        const Polynomial poly = Polynomial::Zero();
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(factory);
        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_HermitianVariable) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_a, 2.0, false}});
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_HermitianVariableComplexFactor) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_a, std::complex{1.0, 2.0}, false}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        const Polynomial expected_poly = factory({Monomial{this->id_a, 1.0, false}});

        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, expected_poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_HermitianString) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_a, 2.0, false}, Monomial{this->id_b, -3.0, false}});
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_NonHermitianVariable) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_ab, 2.0, false}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        const Polynomial expected_poly = factory({Monomial{this->id_ab, 1.0, false}, Monomial{this->id_ab, 1.0, true}});

        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, expected_poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_NonHermitianVariableComplexFactor) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_ab, std::complex{1.0, 2.0}, false}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        const Polynomial expected_poly = factory({Monomial{this->id_ab, std::complex{0.5, 1.0}, false},
                                                  Monomial{this->id_ab, std::complex{0.5, -1.0}, true}});

        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, expected_poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_NonHermitianString) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_ab, std::complex{1.0, 2.0}, false},
                                         Monomial{this->id_ab, std::complex{5.0, -6.0}, true}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        const Polynomial expected_poly = factory({Monomial{this->id_ab, std::complex{3.0, 4.0}, false},
                                                  Monomial{this->id_ab, std::complex{3.0, -4.0}, true}});

        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, expected_poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Real_AntiHermitianString) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_ab, std::complex{1.0, 2.0}, false},
                                         Monomial{this->id_ab, std::complex{-1.0, 2.0}, true}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial real_poly = poly.Real(this->get_factory());
        const Polynomial expected_poly = Polynomial::Zero();

        EXPECT_TRUE(real_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(real_poly, expected_poly, factory.zero_tolerance);
    }



    TEST_F(Symbolic_Polynomial_ReIm, Imaginary_Empty) {
        const auto& factory = this->get_factory();
        const Polynomial poly = Polynomial::Zero();
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial imaginary_poly = poly.Imaginary(factory);
        EXPECT_TRUE(imaginary_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(imaginary_poly, poly, factory.zero_tolerance);
    }


    TEST_F(Symbolic_Polynomial_ReIm, Imaginary_HermitianVariable) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_a, 2.0, false}});
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial imaginary_poly = poly.Imaginary(this->get_factory());
        expect_approximately_equal(imaginary_poly, Polynomial::Zero(), factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Imaginary_HermitianVariableComplexFactor) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_a, std::complex{1.0, 2.0}, false}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial imaginary_poly = poly.Imaginary(this->get_factory());
        const Polynomial expected_poly = factory({Monomial{this->id_a, std::complex{2.0, 0.0}, false}});

        EXPECT_TRUE(imaginary_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(imaginary_poly, expected_poly, factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Imaginary_HermitianString) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_a, 2.0, false}, Monomial{this->id_b, -3.0, false}});
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial imaginary_poly = poly.Imaginary(this->get_factory());
        EXPECT_TRUE(imaginary_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(imaginary_poly, Polynomial::Zero(), factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Imaginary_HermitianStringTwo) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_ab, {0.0, 2.0}, false},
                                         Monomial{this->id_ab, std::complex{0.0, -2.0}, true}});
        EXPECT_TRUE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial imaginary_poly = poly.Imaginary(this->get_factory());
        EXPECT_TRUE(imaginary_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        expect_approximately_equal(imaginary_poly, Polynomial::Zero(), factory.zero_tolerance);
    }

    TEST_F(Symbolic_Polynomial_ReIm, Imaginary_AntiHermitianString) {
        const auto& factory = this->get_factory();
        const Polynomial poly = factory({Monomial{this->id_ab, {1.0, 0.0}, false},
                                         Monomial{this->id_ab, std::complex{-1.0, 0.0}, true}});
        EXPECT_FALSE(poly.is_hermitian(factory.symbols, factory.zero_tolerance));

        const Polynomial imaginary_poly = poly.Imaginary(this->get_factory());
        EXPECT_TRUE(imaginary_poly.is_hermitian(factory.symbols, factory.zero_tolerance));
        const Polynomial expected_poly = factory({Monomial{this->id_ab, {0.0, -1.0}, false},
                                                  Monomial{this->id_ab, std::complex{0.0, +1.0}, true}});

        expect_approximately_equal(imaginary_poly, expected_poly, factory.zero_tolerance);
    }
}