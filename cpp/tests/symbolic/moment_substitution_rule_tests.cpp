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

        MomentSubstitutionRule msr{table, SymbolCombo::Zero()};

        EXPECT_EQ(msr.LHS(), 0);
        EXPECT_EQ(msr.RHS(), SymbolCombo::Zero());
        EXPECT_TRUE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ThreeToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        SymbolCombo combo{Monomial{3, 1.0}}; // #2 + 0.5 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), SymbolCombo::Zero());
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_TwoToScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        SymbolCombo combo{Monomial{2, 1.0}, Monomial{1, -0.5}}; // #2 + 0.5 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 2);
        EXPECT_EQ(msr.RHS(), SymbolCombo::Scalar(0.5));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ThreeToTwoPlusOne) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        SymbolCombo combo{Monomial{3, -1.0}, Monomial{2, 1.0},
                          Monomial{1, 1.0}}; // -#3 + #2 + 1 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), SymbolCombo({Monomial{2, 1.0}, Monomial{1, 1.0}}));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_HalfThreeStarToTwo) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        SymbolCombo combo{Monomial{3, 0.5, true}, Monomial{2, 1.0}}; // 0.5#3* + #2 = 0
        MomentSubstitutionRule msr{table, std::move(combo)};

        EXPECT_EQ(msr.LHS(), 3);
        EXPECT_EQ(msr.RHS(), SymbolCombo(Monomial{2, -2.0, true}));
        EXPECT_FALSE(msr.is_trivial());
    }

    TEST(Symbolic_MomentSubstitutionRule, FromPolynomial_ErrorBadScalar) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        SymbolCombo combo{Monomial{1, 2.5}}; // #2 + 0.5 = 0
        EXPECT_THROW([[maybe_unused]] auto msr = MomentSubstitutionRule(table, std::move(combo)),
                     errors::invalid_moment_rule);
    }


    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToZero) {
        // Fake context/table with 4 non-trivial symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);
        SymbolComboFactory factory{table};

        MomentSubstitutionRule msr{2, SymbolCombo::Zero()}; // #2 -> 0.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), SymbolCombo::Zero());

        const SymbolCombo input_two{{Monomial(2, 1.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(factory, input_two), SymbolCombo::Zero());

        const SymbolCombo input_two_plus_scalar{{Monomial{2, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_two_plus_scalar), SymbolCombo::Scalar(3.0));

        const SymbolCombo input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two), SymbolCombo({Monomial{3, 1.0}}));

        const SymbolCombo input_two_plus_two_star{{Monomial(2, 1.0), Monomial(2, 1.0, true)}};
        EXPECT_EQ(msr.reduce(factory, input_two_plus_two_star), SymbolCombo::Zero());

        const SymbolCombo noMatch{{Monomial{3, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToScalar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        SymbolComboFactory factory{table};

        MomentSubstitutionRule msr{2, SymbolCombo::Scalar(0.5)}; // #2 -> 0.5#1.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), SymbolCombo::Scalar(0.5));

        const SymbolCombo input_two{{Monomial(2, 2.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(factory, input_two), SymbolCombo::Scalar(1.0));

        const SymbolCombo input_two_conj{{Monomial(2, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_two_conj));
        EXPECT_EQ(msr.reduce(factory, input_two_conj), SymbolCombo::Scalar(1.0));

        const SymbolCombo input_two_plus_scalar{{Monomial{2, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_two_plus_scalar), SymbolCombo::Scalar(3.5));

        const SymbolCombo input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  SymbolCombo({Monomial{3, 1.0}, Monomial{1, 1.5}}));

        const SymbolCombo input_two_minus_half{{Monomial{2, 1.0}, Monomial{1, -0.5}}};
        EXPECT_TRUE(msr.matches(input_two_minus_half));
        EXPECT_EQ(msr.reduce(factory, input_two_minus_half), SymbolCombo::Zero());

        const SymbolCombo input_two_plus_two_star{{Monomial{2, 1.0}, Monomial{2, 1.0, true}}};
        EXPECT_EQ(msr.reduce(factory, input_two_plus_two_star), SymbolCombo::Scalar(1.0));

        const SymbolCombo noMatch{{Monomial{3, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwo) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        SymbolComboFactory factory{table};

        MomentSubstitutionRule msr{3, SymbolCombo(Monomial{2, 1.0})}; // #3 -> #2
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), SymbolCombo(Monomial{2, 1.0}));

        const SymbolCombo input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), SymbolCombo(Monomial{2, 2.0}));

        const SymbolCombo input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj), SymbolCombo(Monomial{2, 2.0, true}));

        const SymbolCombo input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  SymbolCombo({Monomial{2, 1.0}, Monomial{1, 3.0}}));

        const SymbolCombo input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two), SymbolCombo({Monomial{2, 4.0}}));

        const SymbolCombo input_three_minus_two{{Monomial{3, 1.0}, Monomial{2, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_two), SymbolCombo::Zero());

        const SymbolCombo input_three_plus_three_star{{Monomial{3, 1.0}, Monomial{3, 1.0, true}}};
        EXPECT_EQ(msr.reduce(factory, input_three_plus_three_star), SymbolCombo({Monomial{2, 1.0},
                                                                                 Monomial{2, 1.0, true}}));

        const SymbolCombo noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);


    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToHalfTwoStar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        SymbolComboFactory factory{table};

        MomentSubstitutionRule msr{3, SymbolCombo(Monomial{2, 0.5, true})}; // #3 -> 0.5#2*.
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), SymbolCombo(Monomial{2, 0.5, true}));

        const SymbolCombo input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), SymbolCombo(Monomial{2, 1.0, true}));

        const SymbolCombo input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj), SymbolCombo(Monomial{2, 1.0, false}));

        const SymbolCombo input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  SymbolCombo({Monomial{2, 0.5, true}, Monomial{1, 3.0}}));

        const SymbolCombo input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  SymbolCombo({Monomial{2, 3.0}, Monomial{2, 0.5, true}}));

        const SymbolCombo input_three_minus_half_two_star{{Monomial{3, 1.0}, Monomial{2, -0.5, true}}};
        EXPECT_TRUE(msr.matches(input_three_minus_half_two_star));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_half_two_star), SymbolCombo::Zero());

        const SymbolCombo noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwoPlusOne) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);
        SymbolComboFactory factory{table};


        MomentSubstitutionRule msr{3,
                                   SymbolCombo({Monomial{2, 1.0}, Monomial{1, 1.0}})}; // #3 -> #2 + 1
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), SymbolCombo({Monomial{2, 1.0}, Monomial{1, 1.0}}));

        const SymbolCombo input_three{{Monomial(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(factory, input_three), SymbolCombo({Monomial{2, 2.0}, Monomial{1, 2.0}}));

        const SymbolCombo input_three_conj{{Monomial(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(factory, input_three_conj),
                  SymbolCombo({Monomial{2, 2.0, true}, Monomial{1, 2.0}}));

        const SymbolCombo input_three_plus_scalar{{Monomial{3, 1.0}, Monomial{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_scalar),
                  SymbolCombo({Monomial{2, 1.0}, Monomial{1, 4.0}}));

        const SymbolCombo input_three_plus_two{{Monomial{3, 1.0}, Monomial{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_three_plus_two),
                  SymbolCombo({Monomial{2, 4.0}, Monomial{1, 1.0}}));

        const SymbolCombo input_three_minus_two_minus_one{{Monomial{3, 1.0},
                                                           Monomial{2, -1.0},
                                                           Monomial{1, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two_minus_one));
        EXPECT_EQ(msr.reduce(factory, input_three_minus_two_minus_one), SymbolCombo::Zero());

        const SymbolCombo noMatch{{Monomial{2, 1.0}, Monomial{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_WithOpOrderFactory) {
        // Fake context/table with 4 non-trivial symbols
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto &table = ams.Symbols();
        ams.generate_dictionary(2); // 0, 1, a, b, aa, ab, (ba), bb

        ByHashSymbolComboFactory factory{table};

        MomentSubstitutionRule msr{5, factory({Monomial{2, 0.5}})}; // #5 -> 0.5#2 (<ab> -> <a>).
        ASSERT_EQ(msr.LHS(), 5);
        ASSERT_EQ(msr.RHS(), factory({Monomial{2, 0.5}}));

        const SymbolCombo input_five = factory({{Monomial(5, 2.0)}});
        EXPECT_TRUE(msr.matches(input_five));
        EXPECT_EQ(msr.reduce(factory, input_five), factory({Monomial{2, 1.0}}));

        const SymbolCombo input_five_conj = factory({{Monomial(5, 2.0, true)}});
        EXPECT_TRUE(msr.matches(input_five_conj));
        EXPECT_EQ(msr.reduce(factory, input_five_conj),
                  factory({Monomial{2, 1.0, false}}));

        const SymbolCombo input_five_plus_scalar =
                factory({Monomial{5, 1.0}, Monomial{1, 3.0}});
        EXPECT_TRUE(msr.matches(input_five_plus_scalar));
        EXPECT_EQ(msr.reduce(factory, input_five_plus_scalar),
                factory({Monomial{2, 0.5}, Monomial{1, 3.0}}));

        const SymbolCombo input_five_plus_two
            = factory({Monomial{5, 1.0}, Monomial{2, 3.0}});
        EXPECT_TRUE(msr.matches(input_five_plus_two));
        EXPECT_EQ(msr.reduce(factory, input_five_plus_two),
                  factory({Monomial{2, 3.5}}));

        const SymbolCombo input_five_minus_half_two
            = factory({{Monomial{5, 1.0}, Monomial{2, -0.5}}});
        EXPECT_TRUE(msr.matches(input_five_minus_half_two));
        EXPECT_EQ(msr.reduce(factory, input_five_minus_half_two), SymbolCombo::Zero());

        const SymbolCombo noMatch = factory({{Monomial{2, 1.0}, Monomial{4, -1.0}}});
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(factory, noMatch), noMatch);
    }

}
