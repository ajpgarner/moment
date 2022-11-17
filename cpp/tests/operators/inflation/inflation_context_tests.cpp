/**
 * inflation_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

namespace NPATK::Tests {

    TEST(InflationContext, Construct_Empty) {
        InflationContext ic{CausalNetwork{{}, {}}, 1};
        EXPECT_EQ(ic.size(), 0);
    }

    TEST(InflationContext, Construct_Pair) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 1};
        ASSERT_EQ(ic.size(), 3);

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 3);
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].sources.contains(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 2);
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].sources.contains(0));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));

    }

    TEST(InflationContext, NumberOperators) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 2};
        ASSERT_EQ(ic.size(), 6);
        ASSERT_EQ(ic.Observables()[0].count_copies(2), 2);
        ASSERT_EQ(ic.Observables()[1].count_copies(2), 2);
        ASSERT_EQ(ic.Observables()[0].count_operators(2), 4);
        ASSERT_EQ(ic.Observables()[1].count_operators(2), 2);
        auto A0_0 = ic.operator_number(0, 0, 0);
        auto A0_1 = ic.operator_number(0, 0, 1);
        auto A1_0 = ic.operator_number(0, 1, 0);
        auto A1_1 = ic.operator_number(0, 1, 1);
        auto B_0 = ic.operator_number(1, 0, 0);
        auto B_1 = ic.operator_number(1, 0, 1);
        std::set<oper_name_t> found_opers = {A0_0, A0_1, A1_0, A1_1, B_0, B_1};
        ASSERT_EQ(found_opers.size(), 6);
    }

    TEST(InflationContext, Sequence_Commute) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 2};

        auto A0_0 = ic.operator_number(0, 0, 0);
        auto A0_1 = ic.operator_number(0, 0, 1);
        auto A1_0 = ic.operator_number(0, 1, 0);
        auto A1_1 = ic.operator_number(0, 1, 1);
        auto B_0 = ic.operator_number(1, 0, 0);

        EXPECT_EQ(OperatorSequence({B_0, A0_0}, ic), OperatorSequence({A0_0, B_0}, ic));
        EXPECT_EQ(OperatorSequence({B_0, A0_1}, ic), OperatorSequence({A0_1, B_0}, ic));
        EXPECT_EQ(OperatorSequence({B_0, A1_0}, ic), OperatorSequence({A1_0, B_0}, ic));
        EXPECT_EQ(OperatorSequence({B_0, A1_1}, ic), OperatorSequence({A1_1, B_0}, ic));
    }

    TEST(InflationContext, Sequence_Orthogonal) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 2};

        auto A0_0 = ic.operator_number(0, 0, 0);
        auto A0_1 = ic.operator_number(0, 0, 1);
        auto A1_0 = ic.operator_number(0, 1, 0);
        auto A1_1 = ic.operator_number(0, 1, 1);

        EXPECT_EQ(OperatorSequence({A0_0, A0_1}, ic), OperatorSequence::Zero(ic));
        EXPECT_EQ(OperatorSequence({A1_0, A1_1}, ic), OperatorSequence::Zero(ic));
        EXPECT_NE(OperatorSequence({A0_0, A1_1}, ic), OperatorSequence::Zero(ic));
        EXPECT_NE(OperatorSequence({A0_1, A1_0}, ic), OperatorSequence::Zero(ic));
    }

    TEST(InflationContext, Sequence_Projector) {
        InflationContext ic{CausalNetwork{{2, 2}, {{0, 1}}}, 2};

        auto A0 = ic.operator_number(0, 0, 0);
        auto A1 = ic.operator_number(0, 1, 0);

        EXPECT_EQ(OperatorSequence({A0, A0}, ic), OperatorSequence({A0}, ic));
        EXPECT_EQ(OperatorSequence({A0, A0, A0}, ic), OperatorSequence({A0}, ic));
        OperatorSequence three{{A0, A0, A1}, ic};
        ASSERT_EQ(three.size(), 2);
        EXPECT_EQ(three[0], A0);
        EXPECT_EQ(three[1], A1);
        EXPECT_EQ(OperatorSequence({A0, A0, A1}, ic), OperatorSequence({A0, A1}, ic));

    }

}
