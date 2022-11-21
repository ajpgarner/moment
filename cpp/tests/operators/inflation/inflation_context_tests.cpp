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

    TEST(InflationContext, ObservableVariants_Pair) {
        InflationContext ic{CausalNetwork{{2, 2}, {{0, 1}}}, 2};
        ASSERT_EQ(ic.Observables().size(), 2);
        const auto& obsA = ic.Observables()[0];
        EXPECT_EQ(obsA.variant_count, 2);
        ASSERT_EQ(obsA.variants.size(), 2);

        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0});
        EXPECT_EQ(obsA_V0.flat_index, 0);
        EXPECT_EQ(obsA_V0.source_variants.size(), 1);
        auto A0_sv_iter = obsA_V0.source_variants.begin();
        ASSERT_NE(A0_sv_iter, obsA_V0.source_variants.end());
        EXPECT_EQ(A0_sv_iter->first, 0);
        EXPECT_EQ(A0_sv_iter->second, 0);

        const auto& obsA_V1 = obsA.variant(std::vector<oper_name_t>{1});
        EXPECT_EQ(obsA_V1.flat_index, 1);
        EXPECT_EQ(obsA_V0.source_variants.size(), 1);
        auto A1_sv_iter = obsA_V1.source_variants.begin();
        ASSERT_NE(A1_sv_iter, obsA_V1.source_variants.end());
        EXPECT_EQ(A1_sv_iter->first, 0);
        EXPECT_EQ(A1_sv_iter->second, 1);

        const auto& obsB = ic.Observables()[1];
        EXPECT_EQ(obsB.variant_count, 2);
        ASSERT_EQ(obsB.variants.size(), 2);

        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0});
        EXPECT_EQ(obsB_V0.flat_index, 0);
        EXPECT_EQ(obsA_V0.source_variants.size(), 1);
        auto B0_sv_iter = obsB_V0.source_variants.begin();
        ASSERT_NE(B0_sv_iter, obsB_V0.source_variants.end());
        EXPECT_EQ(B0_sv_iter->first, 0);
        EXPECT_EQ(B0_sv_iter->second, 0);

        const auto& obsB_V1 = obsB.variant(std::vector<oper_name_t>{1});
        EXPECT_EQ(obsB_V1.flat_index, 1);
        EXPECT_EQ(obsA_V0.source_variants.size(), 1);
        auto B1_sv_iter = obsB_V1.source_variants.begin();
        ASSERT_NE(B1_sv_iter, obsB_V1.source_variants.end());
        EXPECT_EQ(B1_sv_iter->first, 0);
        EXPECT_EQ(B1_sv_iter->second, 1);
    }

    TEST(InflationContext, ObservableVariants_Triangle) {
        InflationContext ic{CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}}, 2};
        ASSERT_EQ(ic.Observables().size(), 3);
        const auto& obsA = ic.Observables()[0];
        EXPECT_EQ(obsA.variant_count, 4);
        ASSERT_EQ(obsA.variants.size(), 4);

        // A, V0
        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0, 0});
        EXPECT_EQ(obsA_V0.flat_index, 0);
        EXPECT_EQ(obsA_V0.source_variants.size(), 2);
        auto A0_sv_iter = obsA_V0.source_variants.begin();
        ASSERT_NE(A0_sv_iter, obsA_V0.source_variants.end());
        EXPECT_EQ(A0_sv_iter->first, 0);
        EXPECT_EQ(A0_sv_iter->second, 0);
        ++A0_sv_iter;
        ASSERT_NE(A0_sv_iter, obsA_V0.source_variants.end());
        EXPECT_EQ(A0_sv_iter->first, 2);
        EXPECT_EQ(A0_sv_iter->second, 0);

        // A, V1
        const auto& obsA_V1 = obsA.variant(std::vector<oper_name_t>{0, 1});
        EXPECT_EQ(obsA_V1.flat_index, 1);
        EXPECT_EQ(obsA_V1.source_variants.size(), 2);
        auto A1_sv_iter = obsA_V1.source_variants.begin();
        ASSERT_NE(A1_sv_iter, obsA_V1.source_variants.end());
        EXPECT_EQ(A1_sv_iter->first, 0);
        EXPECT_EQ(A1_sv_iter->second, 0);
        ++A1_sv_iter;
        ASSERT_NE(A1_sv_iter, obsA_V1.source_variants.end());
        EXPECT_EQ(A1_sv_iter->first, 2);
        EXPECT_EQ(A1_sv_iter->second, 1);

        // A, V2
        const auto& obsA_V2 = obsA.variant(std::vector<oper_name_t>{1, 0});
        EXPECT_EQ(obsA_V2.flat_index, 2);
        EXPECT_EQ(obsA_V2.source_variants.size(), 2);
        auto A2_sv_iter = obsA_V2.source_variants.begin();
        ASSERT_NE(A2_sv_iter, obsA_V2.source_variants.end());
        EXPECT_EQ(A2_sv_iter->first, 0);
        EXPECT_EQ(A2_sv_iter->second, 1);
        ++A2_sv_iter;
        ASSERT_NE(A2_sv_iter, obsA_V2.source_variants.end());
        EXPECT_EQ(A2_sv_iter->first, 2);
        EXPECT_EQ(A2_sv_iter->second, 0);

        // A, V3
        const auto& obsA_V3 = obsA.variant(std::vector<oper_name_t>{1, 1});
        EXPECT_EQ(obsA_V3.flat_index, 3);
        EXPECT_EQ(obsA_V3.source_variants.size(), 2);
        auto A3_sv_iter = obsA_V3.source_variants.begin();
        ASSERT_NE(A3_sv_iter, obsA_V3.source_variants.end());
        EXPECT_EQ(A3_sv_iter->first, 0);
        EXPECT_EQ(A3_sv_iter->second, 1);
        ++A3_sv_iter;
        ASSERT_NE(A3_sv_iter, obsA_V3.source_variants.end());
        EXPECT_EQ(A3_sv_iter->first, 2);
        EXPECT_EQ(A3_sv_iter->second, 1);
    }

    TEST(InflationContext, ObservableIndependence_Pair) {
        InflationContext ic{CausalNetwork{{2, 2}, {{0, 1}}}, 2};
        const auto& obsA = ic.Observables()[0];
        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0});
        const auto& obsA_V1 = obsA.variant(std::vector<oper_name_t>{1});

        const auto& obsB = ic.Observables()[1];
        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0});
        const auto& obsB_V1 = obsB.variant(std::vector<oper_name_t>{1});

        // A0
        EXPECT_FALSE(obsA_V0.independent(obsA_V0));
        EXPECT_TRUE(obsA_V0.independent(obsA_V1));
        EXPECT_FALSE(obsA_V0.independent(obsB_V0));
        EXPECT_TRUE(obsA_V0.independent(obsB_V1));

        // A1
        EXPECT_TRUE(obsA_V1.independent(obsA_V0));
        EXPECT_FALSE(obsA_V1.independent(obsA_V1));
        EXPECT_TRUE(obsA_V1.independent(obsB_V0));
        EXPECT_FALSE(obsA_V1.independent(obsB_V1));

        // B0
        EXPECT_FALSE(obsB_V0.independent(obsA_V0));
        EXPECT_TRUE(obsB_V0.independent(obsA_V1));
        EXPECT_FALSE(obsB_V0.independent(obsB_V0));
        EXPECT_TRUE(obsB_V0.independent(obsB_V1));

        // B1
        EXPECT_TRUE(obsB_V1.independent(obsA_V0));
        EXPECT_FALSE(obsB_V1.independent(obsA_V1));
        EXPECT_TRUE(obsB_V1.independent(obsB_V0));
        EXPECT_FALSE(obsB_V1.independent(obsB_V1));
    }

    TEST(InflationContext, ObservableIndependence_Triangle) {
        InflationContext ic{CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}}, 2};
        ASSERT_EQ(ic.Observables().size(), 3);

        const auto& obsA = ic.Observables()[0]; // Source 0 and 2
        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0,0});
        const auto& obsA_V1 = obsA.variant(std::vector<oper_name_t>{0,1});
        const auto& obsA_V2 = obsA.variant(std::vector<oper_name_t>{1,0});
        const auto& obsA_V3 = obsA.variant(std::vector<oper_name_t>{1,1});

        const auto& obsB = ic.Observables()[1]; // Source 0 and 1
        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0,0});
        const auto& obsB_V1 = obsB.variant(std::vector<oper_name_t>{0,1});
        const auto& obsB_V2 = obsB.variant(std::vector<oper_name_t>{1,0});
        const auto& obsB_V3 = obsB.variant(std::vector<oper_name_t>{1,1});

        const auto& obsC = ic.Observables()[2]; // Source 1 and 2
        const auto& obsC_V0 = obsC.variant(std::vector<oper_name_t>{0,0});
        const auto& obsC_V1 = obsC.variant(std::vector<oper_name_t>{0,1});
        const auto& obsC_V2 = obsC.variant(std::vector<oper_name_t>{1,0});
        const auto& obsC_V3 = obsC.variant(std::vector<oper_name_t>{1,1});


        // A <-> B: shared source 0 [first of A, first of B]
        EXPECT_FALSE(obsA_V0.independent(obsB_V0));
        EXPECT_FALSE(obsA_V0.independent(obsB_V1));
        EXPECT_TRUE(obsA_V0.independent(obsB_V2));
        EXPECT_TRUE(obsA_V0.independent(obsB_V3));

        EXPECT_FALSE(obsA_V1.independent(obsB_V0));
        EXPECT_FALSE(obsA_V1.independent(obsB_V1));
        EXPECT_TRUE(obsA_V1.independent(obsB_V2));
        EXPECT_TRUE(obsA_V1.independent(obsB_V3));

        EXPECT_TRUE(obsA_V2.independent(obsB_V0));
        EXPECT_TRUE(obsA_V2.independent(obsB_V1));
        EXPECT_FALSE(obsA_V2.independent(obsB_V2));
        EXPECT_FALSE(obsA_V2.independent(obsB_V3));

        EXPECT_TRUE(obsA_V3.independent(obsB_V0));
        EXPECT_TRUE(obsA_V3.independent(obsB_V1));
        EXPECT_FALSE(obsA_V3.independent(obsB_V2));
        EXPECT_FALSE(obsA_V3.independent(obsB_V3));

        // A <-> C: shared source 2 [second of A, second of C]
        EXPECT_FALSE(obsA_V0.independent(obsC_V0));
        EXPECT_TRUE(obsA_V0.independent(obsC_V1));
        EXPECT_FALSE(obsA_V0.independent(obsC_V2));
        EXPECT_TRUE(obsA_V0.independent(obsC_V3));

        EXPECT_TRUE(obsA_V1.independent(obsC_V0));
        EXPECT_FALSE(obsA_V1.independent(obsC_V1));
        EXPECT_TRUE(obsA_V1.independent(obsC_V2));
        EXPECT_FALSE(obsA_V1.independent(obsC_V3));

        EXPECT_FALSE(obsA_V2.independent(obsC_V0));
        EXPECT_TRUE(obsA_V2.independent(obsC_V1));
        EXPECT_FALSE(obsA_V2.independent(obsC_V2));
        EXPECT_TRUE(obsA_V2.independent(obsC_V3));

        EXPECT_TRUE(obsA_V3.independent(obsC_V0));
        EXPECT_FALSE(obsA_V3.independent(obsC_V1));
        EXPECT_TRUE(obsA_V3.independent(obsC_V2));
        EXPECT_FALSE(obsA_V3.independent(obsC_V3));

        // B <-> C: shared source 1 [second of B, first of C]
        EXPECT_FALSE(obsB_V0.independent(obsC_V0));
        EXPECT_FALSE(obsB_V0.independent(obsC_V1));
        EXPECT_TRUE(obsB_V0.independent(obsC_V2));
        EXPECT_TRUE(obsB_V0.independent(obsC_V3));

        EXPECT_TRUE(obsB_V1.independent(obsC_V0));
        EXPECT_TRUE(obsB_V1.independent(obsC_V1));
        EXPECT_FALSE(obsB_V1.independent(obsC_V2));
        EXPECT_FALSE(obsB_V1.independent(obsC_V3));

        EXPECT_FALSE(obsB_V2.independent(obsC_V0));
        EXPECT_FALSE(obsB_V2.independent(obsC_V1));
        EXPECT_TRUE(obsB_V2.independent(obsC_V2));
        EXPECT_TRUE(obsB_V2.independent(obsC_V3));

        EXPECT_TRUE(obsB_V3.independent(obsC_V0));
        EXPECT_TRUE(obsB_V3.independent(obsC_V1));
        EXPECT_FALSE(obsB_V3.independent(obsC_V2));
        EXPECT_FALSE(obsB_V3.independent(obsC_V3));

    }

}
