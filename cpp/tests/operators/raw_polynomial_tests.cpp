/**
 * raw_polynomial_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "dictionary/raw_polynomial.h"
#include "matrix_system/matrix_system.h"
#include "scenarios/context.h"
#include "symbolic/polynomial.h"


namespace Moment::Tests {
    TEST(Operators_RawPolynomial, Empty) {
        MatrixSystem system{std::make_unique<Context>(3)};

        RawPolynomial empty;
        EXPECT_EQ(empty.size(), 0);
        Polynomial empty_instantiated = empty.to_polynomial(system.polynomial_factory());
        EXPECT_EQ(empty_instantiated.size(), 0);
        EXPECT_TRUE(empty_instantiated.empty());
    }


    TEST(Operators_RawPolynomial, InstantiateWithoutRegistration) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();
        const auto& poly_factory = system.polynomial_factory();

        RawPolynomial raw_poly;
        raw_poly.emplace_back(OperatorSequence{{0}, context}, 2.0);
        raw_poly.emplace_back(OperatorSequence{{1}, context}, -3.0);
        EXPECT_EQ(raw_poly.size(), 2);

        EXPECT_THROW([[maybe_unused]] auto bad_inst = raw_poly.to_polynomial(poly_factory),
                     errors::unregistered_operator_sequence);

        system.generate_dictionary(1);
        auto whereA = symbols.where(OperatorSequence{{0}, context});
        ASSERT_TRUE(whereA.found());
        auto whereB = symbols.where(OperatorSequence{{1}, context});
        ASSERT_TRUE(whereB.found());

        Polynomial poly = raw_poly.to_polynomial(poly_factory);


        ASSERT_EQ(poly.size(), 2);
        EXPECT_EQ(poly[0], Monomial(whereA->Id(), 2.0));
        EXPECT_EQ(poly[1], Monomial(whereB->Id(), -3.0));

    }

    TEST(Operators_RawPolynomial, InstantiateWithRegistration) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        auto& symbols = system.Symbols();
        const auto& poly_factory = system.polynomial_factory();

        RawPolynomial raw_poly;
        raw_poly.emplace_back(OperatorSequence{{0}, context}, 2.0);
        raw_poly.emplace_back(OperatorSequence{{1}, context}, -3.0);
        EXPECT_EQ(raw_poly.size(), 2);

        Polynomial poly = raw_poly.to_polynomial_register_symbols(poly_factory, symbols);

        auto whereA = symbols.where(OperatorSequence{{0}, context});
        ASSERT_TRUE(whereA.found());
        auto whereB = symbols.where(OperatorSequence{{1}, context});
        ASSERT_TRUE(whereB.found());
        ASSERT_NE(whereA->Id(), whereB->Id());

        ASSERT_EQ(poly.size(), 2);
        EXPECT_EQ(poly[0], Monomial(whereA->Id(), 2.0));
        EXPECT_EQ(poly[1], Monomial(whereB->Id(), -3.0));
    }


    TEST(Operators_RawPolynomial, FromPolynomial) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const auto& symbols = system.Symbols();
        const auto& poly_factory = system.polynomial_factory();

        system.generate_dictionary(1);
        const OperatorSequence os_A{{0}, context};
        auto whereA = symbols.where(os_A);
        ASSERT_TRUE(whereA.found());
        const OperatorSequence os_B{{1}, context};
        auto whereB = symbols.where(os_B);
        ASSERT_TRUE(whereB.found());

        Polynomial poly = poly_factory({{Monomial{whereA->Id(), 2.0}, Monomial{whereB->Id(), {0.0, 4.0}}}});
        ASSERT_EQ(poly.size(), 2);

        RawPolynomial raw_poly{poly, symbols};
        ASSERT_EQ(raw_poly.size(), 2);
        EXPECT_EQ(raw_poly[0].sequence, os_A);
        EXPECT_EQ(raw_poly[0].weight, 2.0);
        EXPECT_EQ(raw_poly[1].sequence, os_B);
        EXPECT_EQ(raw_poly[1].weight, std::complex(0.0, 4.0));
    }


}