/**
 * moment_substitution_rule_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"
#include "symbolic/moment_substitution_rule.h"
#include "symbolic/order_symbols_by_hash.h"

namespace Moment::Tests {
    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_Trivial) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        MomentSubstitutionRule msr{table, Polynomial()};

        EXPECT_EQ(msr.LHS(), 0);
        EXPECT_EQ(msr.RHS(), Polynomial());
        EXPECT_TRUE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ThreeToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        Polynomial combo{Monomial{3, 1.0}}; // #2 + 0.5 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), Polynomial());
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_TwoToScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        Polynomial combo{Monomial{2, 1.0}, Monomial{1, -0.5}}; // #2 + 0.5 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 2);
        EXPECT_EQ(msr.RHS(), Polynomial::Scalar(0.5));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ThreeToTwoPlusOne) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        Polynomial combo{Monomial{3, -1.0}, Monomial{2, 1.0},
                         Monomial{1, 1.0}}; // -#3 + #2 + 1 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), Polynomial({Monomial{2, 1.0}, Monomial{1, 1.0}}));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_HalfThreeStarToTwo) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        Polynomial combo{Monomial{3, 0.5, true}, Monomial{2, 1.0}}; // 0.5#3* + #2 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), Polynomial(Monomial{2, -2.0, true}));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ErrorBadScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        Polynomial combo{Monomial{1, 2.5}}; // #2 + 0.5 = 0
        EXPECT_THROW([[maybe_unused]] auto msr = MomentSubstitutionRule(table, std::move(combo)),
                     errors::invalid_moment_rule);
    }


    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{2, Polynomial()}; // #2 -> 0.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), Polynomial());

        const Polynomial input_two{{Monomial(2, 1.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(factory, input_two), Polynomial());

        const Polynomial input_two_plus_scalar{{Monomial{2, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_two_plus_scalar), Polynomial::Scalar(3.0));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two), Polynomial({Monomial{3, 1.0}}));

        const Polynomial input_two_plus_two_star{{Monomial(2, 1.0), Monomial(2, 1.0, true)}};
        EXPECT_EQ(msr.reduce(factory, input_two_plus_two_star), Polynomial());

        const Polynomial noMatch{{Monomial{3, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToScalar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{2, Polynomial::Scalar(0.5)}; // #2 -> 0.5#1.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), Polynomial::Scalar(0.5));

        const Polynomial input_two{{Monomial(2, 2.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(factory, input_two), Polynomial::Scalar(1.0));

        const Polynomial input_two_conj{{Monomial(2, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_two_conj));
        EXPECT_EQ(msr.reduce(factory, input_two_conj), Polynomial::Scalar(1.0));

        const Polynomial input_two_plus_scalar{{Monomial{2, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_two_plus_scalar), Polynomial::Scalar(3.5));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  Polynomial({Monomial{3, 1.0}, Monomial{1, 1.5}}));

        const Polynomial input_two_minus_half{{Monomial{2, 1.0}, Monomial{1, -0.5}}};
        EXPECT_TRUE(msr.matches(input_two_minus_half));
        EXPECT_EQ(msr.reduce(factory, input_two_minus_half), Polynomial());

        const Polynomial input_two_plus_two_star{{Monomial{2, 1.0}, Monomial{2, 1.0, true}}};
        EXPECT_EQ(msr.reduce(factory, input_two_plus_two_star), Polynomial::Scalar(1.0));

        const Polynomial noMatch{{Monomial{3, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwo) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{3, Polynomial(Monomial{2, 1.0})}; // #3 -> #2
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), Polynomial(Monomial{2, 1.0}));

        const Polynomial input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), Polynomial(Monomial{2, 2.0}));

        const Polynomial input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj), Polynomial(Monomial{2, 2.0, true}));

        const Polynomial input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  Polynomial({Monomial{2, 1.0}, Monomial{1, 3.0}}));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two), Polynomial({Monomial{2, 4.0}}));

        const Polynomial input_three_minus_two{{Monomial{3, 1.0}, Monomial{2, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_two), Polynomial());

        const Polynomial input_three_plus_three_star{{Monomial{3, 1.0}, Monomial{3, 1.0, true}}};
        EXPECT_EQ(msr.reduce(factory, input_three_plus_three_star), Polynomial({Monomial{2, 1.0},
                                                                                Monomial{2, 1.0, true}}));

        const Polynomial noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);


    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToHalfTwoStar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{3, Polynomial(Monomial{2, 0.5, true})}; // #3 -> 0.5#2*.
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), Polynomial(Monomial{2, 0.5, true}));

        const Polynomial input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), Polynomial(Monomial{2, 1.0, true}));

        const Polynomial input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj), Polynomial(Monomial{2, 1.0, false}));

        const Polynomial input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  Polynomial({Monomial{2, 0.5, true}, Monomial{1, 3.0}}));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  Polynomial({Monomial{2, 3.0}, Monomial{2, 0.5, true}}));

        const Polynomial input_three_minus_half_two_star{{Monomial{3, 1.0}, Monomial{2, -0.5, true}}};
        EXPECT_TRUE(msr.matches(input_three_minus_half_two_star));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_half_two_star), Polynomial());

        const Polynomial noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwoPlusOne) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        PolynomialFactory factory{table};


        MomentSubstitutionRule msr{3,
                                   Polynomial({Monomial{2, 1.0}, Monomial{1, 1.0}})}; // #3 -> #2 + 1
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), Polynomial({Monomial{2, 1.0}, Monomial{1, 1.0}}));

        const Polynomial input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), Polynomial({Monomial{2, 2.0}, Monomial{1, 2.0}}));

        const Polynomial input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj),
                  Polynomial({Monomial{2, 2.0, true}, Monomial{1, 2.0}}));

        const Polynomial input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  Polynomial({Monomial{2, 1.0}, Monomial{1, 4.0}}));

        const Polynomial input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  Polynomial({Monomial{2, 4.0}, Monomial{1, 1.0}}));

        const Polynomial input_three_minus_two_minus_one{{Monomial{3, 1.0},
                                                          Monomial{2, -1.0},
                                                          Monomial{1, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two_minus_one));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_two_minus_one), Polynomial());

        const Polynomial noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_WithOpOrderFactory) {
        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        ByHashPolynomialFactory factory{table};

        MomentSubstitutionRule msr{5, factory({Monomial{2, 0.5}})}; // #5 -> 0.5#2 (<ab> -> <a>).
        ASSERT_EQ(msr.LHS(), 5);
        ASSERT_EQ(msr.RHS(), factory({Monomial{2, 0.5}}));

        const Polynomial input_five = factory({{Monomial(5, 2.0)}});
        EXPECT_TRUE(msr.matches(input_five));
        EXPECT_EQ(msr.reduce(factory, input_five), factory({Monomial{2, 1.0}}));

        const Polynomial input_five_conj = factory({{Monomial(5, 2.0, true)}});
        EXPECT_TRUE(msr.matches(input_five_conj));
        EXPECT_EQ(msr.reduce(factory, input_five_conj),
                  factory({Monomial{2, 1.0, false}}));

        const Polynomial input_five_plus_scalar =
                factory({Monomial{5, 1.0}, Monomial{1, 3.0}});
        EXPECT_TRUE(msr.matches(input_five_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_five_plus_scalar),
                factory({Monomial{2, 0.5}, Monomial{1, 3.0}}));

        const Polynomial input_five_plus_two
            = factory({Monomial{5, 1.0}, Monomial{2, 3.0}});
        EXPECT_TRUE(msr.matches(input_five_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_five_plus_two),
                  factory({Monomial{2, 3.5}}));

        const Polynomial input_five_minus_half_two
            = factory({{Monomial{5, 1.0}, Monomial{2, -0.5}}});
        EXPECT_TRUE(msr.matches(input_five_minus_half_two));
        EXPECT_EQ(msr.reduce(factory, input_five_minus_half_two), Polynomial());

        const Polynomial noMatch = factory({{Monomial{2, 1.0}, Monomial{4, -1.0}}});
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_Trivial) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{table, Polynomial::Zero()};

        EXPECT_TRUE(msr.is_trivial());
        EXPECT_EQ(msr.as_polynomial(factory), Polynomial::Zero());
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_ThreeToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{table, factory({Monomial{3, 1.0}})};
        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{3, -1.0}}));
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_TwoToScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{table, factory({Monomial{2, 1.0}, Monomial{1, -0.5}})};

        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{2, -1.0}, Monomial{1, 0.5}}));
    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_ThreeToTwoPlusOne) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{table, factory({Monomial{3, -1.0}, Monomial{2, 1.0}, Monomial{1, 1.0}})};

        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{3, -1.0}, Monomial{2, 1.0}, Monomial{1, 1.0}}));

    }

    TEST(Symbolic_MomentSubstitutionRule, AsPolynomial_HalfThreeStarToTwo) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        PolynomialFactory factory{table};

        MomentSubstitutionRule msr{table, factory({Monomial{3, 0.5, true}, Monomial{2, 1.0}})};
        EXPECT_EQ(msr.as_polynomial(factory), factory({Monomial{3, -1.0}, Monomial{2, -2.0, true}}));
    }
}
