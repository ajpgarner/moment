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


    TEST(Operators_RawPolynomial, Condense_Empty) {
        RawPolynomial raw_poly{};
        ASSERT_TRUE(raw_poly.empty());
        EXPECT_EQ(raw_poly.size(), 0);
        raw_poly.condense();
        ASSERT_TRUE(raw_poly.empty());
        EXPECT_EQ(raw_poly.size(), 0);
    }

    TEST(Operators_RawPolynomial, Condense_Singlet) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        RawPolynomial raw_poly;
        const OperatorSequence os_A{{0}, context};
        raw_poly.emplace_back(os_A, {1.0, -1.0});
        raw_poly.condense();

        ASSERT_EQ(raw_poly.size(), 1);
        EXPECT_EQ(raw_poly[0].sequence, os_A);
        EXPECT_EQ(raw_poly[0].weight, std::complex(1.0, -1.0));
    }

    TEST(Operators_RawPolynomial, Condense_SingletZeroWeight) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        RawPolynomial raw_poly;
        const OperatorSequence os_A{{0}, context};
        raw_poly.emplace_back(os_A, {0.0, 0.0});
        EXPECT_FALSE(raw_poly.empty());
        raw_poly.condense();
        EXPECT_TRUE(raw_poly.empty());
        EXPECT_EQ(raw_poly.size(), 0);
    }

    TEST(Operators_RawPolynomial, Condense_SingletZeroSequence) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        RawPolynomial raw_poly;
        raw_poly.emplace_back(context.zero(), {1.0, 0.0});
        EXPECT_FALSE(raw_poly.empty());
        raw_poly.condense();
        EXPECT_TRUE(raw_poly.empty());
        EXPECT_EQ(raw_poly.size(), 0);
    }

    TEST(Operators_RawPolynomial, Condense_Pair) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const OperatorSequence os_A{{0}, context};
        const OperatorSequence os_B{{1}, context};
        ASSERT_NE(os_A.hash(), os_B.hash());

        RawPolynomial raw_poly;
        raw_poly.emplace_back(os_A, {2.0, 0.0});
        raw_poly.emplace_back(os_B, {0.0, -3.0});
        EXPECT_FALSE(raw_poly.empty());
        raw_poly.condense();
        ASSERT_EQ(raw_poly.size(), 2);
        EXPECT_EQ(raw_poly[0].sequence, os_A);
        EXPECT_EQ(raw_poly[0].weight, std::complex(2.0, 0.0));
        EXPECT_EQ(raw_poly[1].sequence, os_B);
        EXPECT_EQ(raw_poly[1].weight, std::complex(0.0, -3.0));
    }

    TEST(Operators_RawPolynomial, Condense_PairToSinglet) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const OperatorSequence os_A{{0}, context};

        RawPolynomial raw_poly;
        raw_poly.emplace_back(os_A, {2.0, 0.0});
        raw_poly.emplace_back(os_A, {0.0, -3.0});
        ASSERT_EQ(raw_poly.size(), 2);
        raw_poly.condense();
        ASSERT_EQ(raw_poly.size(), 1);
        EXPECT_EQ(raw_poly[0].sequence, os_A);
        EXPECT_EQ(raw_poly[0].weight, std::complex(2.0, -3.0));
    }

    TEST(Operators_RawPolynomial, Condense_PairToZero) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const OperatorSequence os_A{{0}, context};

        RawPolynomial raw_poly;
        raw_poly.emplace_back(os_A, {2.0, 0.0});
        raw_poly.emplace_back(os_A, {-2.0, 0.0});
        EXPECT_FALSE(raw_poly.empty());
        raw_poly.condense();
        EXPECT_TRUE(raw_poly.empty());
        EXPECT_EQ(raw_poly.size(), 0);
    }

    TEST(Operators_RawPolynomial, Condense_ListFourToThree) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const OperatorSequence os_A{{0}, context};
        const OperatorSequence os_B{{1}, context};
        const OperatorSequence os_C{{0, 1}, context};
        ASSERT_NE(os_A.hash(), os_B.hash());
        ASSERT_NE(os_A.hash(), os_C.hash());
        ASSERT_NE(os_B.hash(), os_C.hash());

        RawPolynomial raw_poly;
        raw_poly.emplace_back(os_A, {2.0, 0.0});
        raw_poly.emplace_back(os_B, {0.0, -3.0});
        raw_poly.emplace_back(os_C, {1.0, 0.0});
        raw_poly.emplace_back(os_B, {1.0, 0.0});
        ASSERT_EQ(raw_poly.size(), 4);

        raw_poly.condense();
        ASSERT_EQ(raw_poly.size(), 3);
        EXPECT_EQ(raw_poly[0].sequence, os_A);
        EXPECT_EQ(raw_poly[0].weight, std::complex(2.0, 0.0));
        EXPECT_EQ(raw_poly[1].sequence, os_B);
        EXPECT_EQ(raw_poly[1].weight, std::complex(1.0, -3.0));
        EXPECT_EQ(raw_poly[2].sequence, os_C);
        EXPECT_EQ(raw_poly[2].weight, std::complex(1.0, 0.0));
    }

    TEST(Operators_RawPolynomial, Condense_ListFourToTwo) {
        MatrixSystem system{std::make_unique<Context>(3)};
        const auto& context = system.Context();
        const OperatorSequence os_A{{0}, context};
        const OperatorSequence os_B{{1}, context};
        const OperatorSequence os_C{{0, 1}, context};
        ASSERT_NE(os_A.hash(), os_B.hash());
        ASSERT_NE(os_A.hash(), os_C.hash());
        ASSERT_NE(os_B.hash(), os_C.hash());

        RawPolynomial raw_poly;
        raw_poly.emplace_back(os_A, {2.0, 0.0});
        raw_poly.emplace_back(os_B, {0.0, -3.0});
        raw_poly.emplace_back(os_C, {1.0, 0.0});
        raw_poly.emplace_back(os_B, {0.0, 3.0});
        ASSERT_EQ(raw_poly.size(), 4);

        raw_poly.condense();
        ASSERT_EQ(raw_poly.size(), 2);
        EXPECT_EQ(raw_poly[0].sequence, os_A);
        EXPECT_EQ(raw_poly[0].weight, std::complex(2.0, 0.0));
        EXPECT_EQ(raw_poly[1].sequence, os_C);
        EXPECT_EQ(raw_poly[1].weight, std::complex(1.0, 0.0));
    }


}