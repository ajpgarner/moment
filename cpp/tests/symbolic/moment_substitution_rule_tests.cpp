/**
 * moment_substitution_rule_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/context.h"
#include "symbolic/symbol_table.h"
#include "symbolic/moment_substitution_rule.h"

namespace Moment::Tests {

    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToZero) {
        // Fake context/table with 4 symbols
        Context context{2};
        SymbolTable table{context};
        table.create(4, true, true);

        MomentSubstitutionRule msr{table, 2, SymbolCombo::Zero()}; // #2 -> 0.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), SymbolCombo::Zero());

        const SymbolCombo input_two{{SymbolExpression(2, 1.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(input_two), SymbolCombo::Zero());

        const SymbolCombo input_two_plus_scalar{{SymbolExpression{2, 1.0}, SymbolExpression{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(input_two_plus_scalar), SymbolCombo::Scalar(3.0));

        const SymbolCombo input_three_plus_two{{SymbolExpression{3, 1.0}, SymbolExpression{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(input_three_plus_two), SymbolCombo({SymbolExpression{3, 1.0}}));

        const SymbolCombo noMatch{{SymbolExpression{3, 1.0}, SymbolExpression{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_TwoToScalar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);

        MomentSubstitutionRule msr{table, 2, SymbolCombo::Scalar(0.5)}; // #2 -> 0.5#1.
        ASSERT_EQ(msr.LHS(), 2);
        ASSERT_EQ(msr.RHS(), SymbolCombo::Scalar(0.5));

        const SymbolCombo input_two{{SymbolExpression(2, 2.0)}};
        EXPECT_TRUE(msr.matches(input_two));
        EXPECT_EQ(msr.reduce(input_two), SymbolCombo::Scalar(1.0));

        const SymbolCombo input_two_conj{{SymbolExpression(2, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_two_conj));
        EXPECT_EQ(msr.reduce(input_two_conj), SymbolCombo::Scalar(1.0));

        const SymbolCombo input_two_plus_scalar{{SymbolExpression{2, 1.0}, SymbolExpression{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_two_plus_scalar));
        EXPECT_EQ(msr.reduce(input_two_plus_scalar), SymbolCombo::Scalar(3.5));

        const SymbolCombo input_three_plus_two{{SymbolExpression{3, 1.0}, SymbolExpression{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(input_three_plus_two), SymbolCombo({SymbolExpression{3, 1.0}, SymbolExpression{1, 1.5}}));

        const SymbolCombo input_two_minus_half{{SymbolExpression{2, 1.0}, SymbolExpression{1, -0.5}}};
        EXPECT_TRUE(msr.matches(input_two_minus_half));
        EXPECT_EQ(msr.reduce(input_two_minus_half), SymbolCombo::Zero());

        const SymbolCombo noMatch{{SymbolExpression{3, 1.0}, SymbolExpression{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwo) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);

        MomentSubstitutionRule msr{table, 3, SymbolCombo(SymbolExpression{2, 1.0})}; // #3 -> #2
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), SymbolCombo(SymbolExpression{2, 1.0}));

        const SymbolCombo input_three{{SymbolExpression(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(input_three), SymbolCombo(SymbolExpression{2, 2.0}));

        const SymbolCombo input_three_conj{{SymbolExpression(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(input_three_conj), SymbolCombo(SymbolExpression{2, 2.0, true}));

        const SymbolCombo input_three_plus_scalar{{SymbolExpression{3, 1.0}, SymbolExpression{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(input_three_plus_scalar),
                  SymbolCombo({SymbolExpression{2, 1.0}, SymbolExpression{1, 3.0}}));

        const SymbolCombo input_three_plus_two{{SymbolExpression{3, 1.0}, SymbolExpression{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(input_three_plus_two), SymbolCombo({SymbolExpression{2, 4.0}}));

        const SymbolCombo input_three_minus_two{{SymbolExpression{3, 1.0}, SymbolExpression{2, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two));
        EXPECT_EQ(msr.reduce(input_three_minus_two), SymbolCombo::Zero());

        const SymbolCombo noMatch{{SymbolExpression{2, 1.0}, SymbolExpression{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToHalfTwoStar) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);

        MomentSubstitutionRule msr{table, 3, SymbolCombo(SymbolExpression{2, 0.5, true})}; // #3 -> 0.5#2*.
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), SymbolCombo(SymbolExpression{2, 0.5, true}));

        const SymbolCombo input_three{{SymbolExpression(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(input_three), SymbolCombo(SymbolExpression{2, 1.0, true}));

        const SymbolCombo input_three_conj{{SymbolExpression(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(input_three_conj), SymbolCombo(SymbolExpression{2, 1.0, false}));

        const SymbolCombo input_three_plus_scalar{{SymbolExpression{3, 1.0}, SymbolExpression{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(input_three_plus_scalar),
                  SymbolCombo({SymbolExpression{2, 0.5, true}, SymbolExpression{1, 3.0}}));

        const SymbolCombo input_three_plus_two{{SymbolExpression{3, 1.0}, SymbolExpression{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(input_three_plus_two),
                  SymbolCombo({SymbolExpression{2, 3.0}, SymbolExpression{2, 0.5, true}}));

        const SymbolCombo input_three_minus_half_two_star{{SymbolExpression{3, 1.0}, SymbolExpression{2, -0.5, true}}};
        EXPECT_TRUE(msr.matches(input_three_minus_half_two_star));
        EXPECT_EQ(msr.reduce(input_three_minus_half_two_star), SymbolCombo::Zero());

        const SymbolCombo noMatch{{SymbolExpression{2, 1.0}, SymbolExpression{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(noMatch), noMatch);
    }

    TEST(Symbolic_MomentSubstitutionRule, Reduce_ThreeToTwoPlusOne) {
        // Fake context/table with 4 symbols
        Context contxt{2};
        SymbolTable table{contxt};
        table.create(4, true, true);

        MomentSubstitutionRule msr{table, 3,
                                   SymbolCombo({SymbolExpression{2, 1.0}, SymbolExpression{1, 1.0}})}; // #3 -> #2 + 1
        ASSERT_EQ(msr.LHS(), 3);
        ASSERT_EQ(msr.RHS(), SymbolCombo({SymbolExpression{2, 1.0}, SymbolExpression{1, 1.0}}));

        const SymbolCombo input_three{{SymbolExpression(3, 2.0)}};
        EXPECT_TRUE(msr.matches(input_three));
        EXPECT_EQ(msr.reduce(input_three), SymbolCombo({SymbolExpression{2, 2.0}, SymbolExpression{1, 2.0}}));

        const SymbolCombo input_three_conj{{SymbolExpression(3, 2.0, true)}};
        EXPECT_TRUE(msr.matches(input_three_conj));
        EXPECT_EQ(msr.reduce(input_three_conj), SymbolCombo({SymbolExpression{2, 2.0, true}, SymbolExpression{1, 2.0}}));

        const SymbolCombo input_three_plus_scalar{{SymbolExpression{3, 1.0}, SymbolExpression{1, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_scalar));
        EXPECT_EQ(msr.reduce(input_three_plus_scalar),
                  SymbolCombo({SymbolExpression{2, 1.0}, SymbolExpression{1, 4.0}}));

        const SymbolCombo input_three_plus_two{{SymbolExpression{3, 1.0}, SymbolExpression{2, 3.0}}};
        EXPECT_TRUE(msr.matches(input_three_plus_two));
        EXPECT_EQ(msr.reduce(input_three_plus_two),
                  SymbolCombo({SymbolExpression{2, 4.0}, SymbolExpression{1, 1.0}}));

        const SymbolCombo input_three_minus_two_minus_one{{SymbolExpression{3, 1.0},
                                                           SymbolExpression{2, -1.0},
                                                           SymbolExpression{1, -1.0}}};
        EXPECT_TRUE(msr.matches(input_three_minus_two_minus_one));
        EXPECT_EQ(msr.reduce(input_three_minus_two_minus_one), SymbolCombo::Zero());

        const SymbolCombo noMatch{{SymbolExpression{2, 1.0}, SymbolExpression{4, -1.0}}};
        EXPECT_FALSE(msr.matches(noMatch));
        EXPECT_EQ(msr.reduce(noMatch), noMatch);
    }
}