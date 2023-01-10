/**
 * substitution_list_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "matrix/moment_matrix.h"

#include "symbolic/substitution_list.h"
#include "symbolic/symbol_table.h"
#include "symbolic_matrix_helpers.h"

#include "scenarios/inflation/inflation_context.h"
#include "scenarios/inflation/inflation_matrix_system.h"

namespace Moment::Tests {

    TEST(Symbolic_SubstitutionList, Empty) {
        SubstitutionList empty{{}};

        SymbolExpression test{3, false, false};
        auto passThru = empty.substitute(test);
        EXPECT_EQ(passThru, test);
    }


    TEST(Symbolic_SubstitutionList, Simple) {
        SubstitutionList simple{{{2, 0.5}, {5, 1.3}}};

        auto doNothing = simple.substitute(SymbolExpression{3});
        EXPECT_EQ(doNothing.id, 3);
        EXPECT_EQ(doNothing.factor, 1.0);
        EXPECT_FALSE(doNothing.conjugated);

        auto twoToOne = simple.substitute(SymbolExpression{2, 0.5});
        EXPECT_EQ(twoToOne.id, 1);
        EXPECT_EQ(twoToOne.factor, 0.25);
        EXPECT_FALSE(twoToOne.conjugated);

        auto fiveToOne = simple.substitute(SymbolExpression{5});
        EXPECT_EQ(fiveToOne.id, 1);
        EXPECT_EQ(fiveToOne.factor, 1.3);
        EXPECT_FALSE(fiveToOne.conjugated);
    }

    TEST(Symbolic_SubstitutionList, WithFactors) {
        // Build unlinked pair (uninflated)
        std::unique_ptr<Inflation::InflationContext> icPtr
                = std::make_unique<Inflation::InflationContext>(Inflation::CausalNetwork{{2, 2}, {}}, 1);
        Inflation::InflationMatrixSystem ims{std::move(icPtr)};
        const auto& context = ims.InflationContext();
        const auto& symbols = ims.Symbols();
        const auto& factors = ims.Factors();

        // Get operator names
        ASSERT_EQ(context.Observables().size(), 2);
        auto op_a = context.Observables()[0].operator_offset;
        auto op_b = context.Observables()[1].operator_offset;

        // Make moment matrix, then find symbols
        ims.create_moment_matrix(1);
        auto id_a = find_or_fail(symbols, OperatorSequence{{op_a}, context});
        auto id_b = find_or_fail(symbols, OperatorSequence{{op_b}, context});
        auto id_ab = find_or_fail(symbols, OperatorSequence{{op_a, op_b}, context});
        symbol_name_t free_id = symbols.size() + 5;

        std::set all_symbols{static_cast<symbol_name_t>(1), id_a, id_b, id_ab};
        ASSERT_EQ(all_symbols.size(), 4);

        // Build substitutions of just A
        SubstitutionList a_to_value{{{id_a, 2.0}}};
        a_to_value.infer_substitutions(ims);

        // Non-matching symbol
        auto pass_thru = a_to_value.substitute(SymbolExpression{free_id, 13.0, true});
        EXPECT_EQ(pass_thru.id, free_id);
        EXPECT_EQ(pass_thru.factor, 13.0);
        EXPECT_TRUE(pass_thru.conjugated);

        // Trivial match: 2A -> 4
        auto trivial_a = a_to_value.substitute(SymbolExpression{id_a, 2.0});
        EXPECT_EQ(trivial_a.id, 1);
        EXPECT_EQ(trivial_a.factor, 4.0);
        EXPECT_FALSE(trivial_a.conjugated);

        // Complex match: AB -> 2B
        auto ab_to_b = a_to_value.substitute(SymbolExpression{id_ab});
        EXPECT_EQ(ab_to_b.id, id_b);
        EXPECT_EQ(ab_to_b.factor, 2.0);
        EXPECT_FALSE(ab_to_b.conjugated);

        // Build substitutions of A and B
        SubstitutionList a_b_to_value{{{id_a, 2.0}, {id_b, 3.0}}};
        a_b_to_value.infer_substitutions(ims);

        // Non-matching symbol
        auto pass_thru2 = a_b_to_value.substitute(SymbolExpression{free_id, 13.0, true});
        EXPECT_EQ(pass_thru2.id, free_id);
        EXPECT_EQ(pass_thru2.factor, 13.0);
        EXPECT_TRUE(pass_thru2.conjugated);

        // Trivial match: 2A -> 4
        auto trivial_a2 = a_b_to_value.substitute(SymbolExpression{id_a, 2.0});
        EXPECT_EQ(trivial_a2.id, 1);
        EXPECT_EQ(trivial_a2.factor, 4.0);
        EXPECT_FALSE(trivial_a2.conjugated);

        // Trivial match: 2A -> 4
        auto trivial_b2 = a_b_to_value.substitute(SymbolExpression{id_b, 2.0});
        EXPECT_EQ(trivial_b2.id, 1);
        EXPECT_EQ(trivial_b2.factor, 6.0);
        EXPECT_FALSE(trivial_b2.conjugated);

        // Complex match: AB -> 6
        auto ab_to_value = a_b_to_value.substitute(SymbolExpression{id_ab});
        EXPECT_EQ(ab_to_value.id, 1);
        EXPECT_EQ(ab_to_value.factor, 6.0);
        EXPECT_FALSE(ab_to_value.conjugated);

    }


    TEST(Symbolic_SubstitutionList, CloneMomentMatrix) {
        // Build unlinked pair (uninflated)
        std::unique_ptr<Inflation::InflationContext> icPtr
                = std::make_unique<Inflation::InflationContext>(Inflation::CausalNetwork{{2, 2},
                                                                                         {}}, 1);
        Inflation::InflationMatrixSystem ims{std::move(icPtr)};
        const auto &context = ims.InflationContext();
        const auto &symbols = ims.Symbols();
        const auto &factors = ims.Factors();

        // Get operator names
        ASSERT_EQ(context.Observables().size(), 2);
        auto op_a = context.Observables()[0].operator_offset;
        auto op_b = context.Observables()[1].operator_offset;

        // Make moment matrix, then find symbols
        auto [mm_id, moment_matrix] = ims.create_moment_matrix(1);
        auto id_e = find_or_fail(symbols, OperatorSequence::Identity(context));
        auto id_a = find_or_fail(symbols, OperatorSequence{{op_a}, context});
        auto id_b = find_or_fail(symbols, OperatorSequence{{op_b}, context});
        auto id_ab = find_or_fail(symbols, OperatorSequence{{op_a, op_b}, context});
        symbol_name_t free_id = symbols.size() + 5;

        std::set all_symbols{id_e, id_a, id_b, id_ab};
        ASSERT_EQ(all_symbols.size(), 4);

        compare_symbol_matrices(moment_matrix.SymbolMatrix, {id_e, id_a, id_b,
                                                             id_a, id_a, id_ab,
                                                             id_b, id_ab, id_b});

        // Build substitutions of just A
        SubstitutionList sub_list{{{id_a, 2.0}}};
        sub_list.infer_substitutions(ims);

        // Rewrite moment matrix with known values
        auto [sub_id, sub_matrix] = ims.clone_and_substitute(mm_id, sub_list);

        // Test matrix object is unique
        ASSERT_NE(mm_id, sub_id);
        ASSERT_NE(&moment_matrix, &sub_matrix);

        // Symbol matrix should have with a replaced by 2.0
        ASSERT_EQ(sub_matrix.Dimension(), 3);
        const auto& sub_symbols = sub_matrix.SymbolMatrix;
        EXPECT_EQ(sub_symbols[0][0], SymbolExpression(id_e));
        EXPECT_EQ(sub_symbols[0][1], SymbolExpression(id_e, 2.0));
        EXPECT_EQ(sub_symbols[0][2], SymbolExpression(id_b));
        EXPECT_EQ(sub_symbols[1][0], SymbolExpression(id_e, 2.0));
        EXPECT_EQ(sub_symbols[1][1], SymbolExpression(id_e, 2.0));
        EXPECT_EQ(sub_symbols[1][2], SymbolExpression(id_b, 2.0));
        EXPECT_EQ(sub_symbols[2][0], SymbolExpression(id_b));
        EXPECT_EQ(sub_symbols[2][1], SymbolExpression(id_b, 2.0));
        EXPECT_EQ(sub_symbols[2][2], SymbolExpression(id_b));
    }


}