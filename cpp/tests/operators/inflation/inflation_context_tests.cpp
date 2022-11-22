/**
 * inflation_context_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/inflation/inflation_context.h"
#include "operators/inflation/inflation_matrix_system.h"

namespace NPATK::Tests {
    namespace {
        void expect_factorizes(const InflationContext& ic, std::vector<oper_name_t>&& sequence) {
            OperatorSequence seq{std::move(sequence), ic};
            const size_t size = seq.size();
            auto factors = ic.factorize(seq);
            ASSERT_EQ(factors.size(), size) << "seq = " << seq;
            for (size_t i = 0; i < size; ++i) {
                EXPECT_EQ(factors[i], OperatorSequence({seq[i]}, ic)) << "seq = " << seq;
            }
        }

        void expect_doesnt_factorize(const InflationContext& ic, std::vector<oper_name_t>&& sequence) {
            OperatorSequence seq{std::move(sequence), ic};
            const size_t size = seq.size();
            auto factors = ic.factorize(seq);
            ASSERT_EQ(factors.size(), 1) << "seq = " << seq;
            EXPECT_EQ(factors[0], seq);
        }
    }

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


    TEST(InflationContext, Factorize_Pair) {
        InflationContext ic{CausalNetwork{{2, 2}, {{0, 1}}}, 2};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];

        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0});
        const auto& obsA_V1 = obsA.variant(std::vector<oper_name_t>{1});
        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0});
        const auto& obsB_V1 = obsB.variant(std::vector<oper_name_t>{1});

        auto id_a0 = obsA_V0.operator_offset;
        auto id_a1 = obsA_V1.operator_offset;
        auto id_b0 = obsB_V0.operator_offset;
        auto id_b1 = obsB_V1.operator_offset;

        // 0, I, a0, a1, b0 and b1 should all just pass through
        auto factors_0 = ic.factorize(OperatorSequence::Zero(ic));
        ASSERT_EQ(factors_0.size(), 1);
        EXPECT_EQ(factors_0[0], OperatorSequence::Zero(ic));

        auto factors_I = ic.factorize(OperatorSequence::Identity(ic));
        ASSERT_EQ(factors_I.size(), 1);
        EXPECT_EQ(factors_I[0], OperatorSequence::Identity(ic));

        auto factors_a0 = ic.factorize(OperatorSequence{{id_a0}, ic});
        ASSERT_EQ(factors_a0.size(), 1);
        EXPECT_EQ(factors_a0[0], OperatorSequence({id_a0}, ic));

        auto factors_a1 = ic.factorize(OperatorSequence{{id_a1}, ic});
        ASSERT_EQ(factors_a1.size(), 1);
        EXPECT_EQ(factors_a1[0], OperatorSequence({id_a1}, ic));

        auto factors_b0 = ic.factorize(OperatorSequence{{id_b0}, ic});
        ASSERT_EQ(factors_b0.size(), 1);
        EXPECT_EQ(factors_b0[0], OperatorSequence({id_b0}, ic));

        auto factors_b1 = ic.factorize(OperatorSequence{{id_b1}, ic});
        ASSERT_EQ(factors_b1.size(), 1);
        EXPECT_EQ(factors_b1[0], OperatorSequence({id_b1}, ic));

        // A0B0 should not factorize, due to common source
        auto factors_a0b0 = ic.factorize(OperatorSequence{{id_a0, id_b0}, ic});
        ASSERT_EQ(factors_a0b0.size(), 1);
        EXPECT_EQ(factors_a0b0[0], OperatorSequence({id_a0, id_b0}, ic));

        // A0B1 should freely factorize
        auto factors_a0b1 = ic.factorize(OperatorSequence{{id_a0, id_b1}, ic});
        ASSERT_EQ(factors_a0b1.size(), 2);
        EXPECT_EQ(factors_a0b1[0], OperatorSequence({id_a0}, ic));
        EXPECT_EQ(factors_a0b1[1], OperatorSequence({id_b1}, ic));

        // A1B0 should freely factorize
        auto factors_a1b0 = ic.factorize(OperatorSequence{{id_a1, id_b0}, ic});
        ASSERT_EQ(factors_a1b0.size(), 2);
        EXPECT_EQ(factors_a1b0[0], OperatorSequence({id_a1}, ic));
        EXPECT_EQ(factors_a1b0[1], OperatorSequence({id_b0}, ic));

        // A1B1 should not factorize, due to common source
        auto factors_a1b1 = ic.factorize(OperatorSequence{{id_a1, id_b1}, ic});
        ASSERT_EQ(factors_a1b1.size(), 1);
        EXPECT_EQ(factors_a1b1[0], OperatorSequence({id_a1, id_b1}, ic));

         // A0A1 should freely factorize
        auto factors_a0a1 = ic.factorize(OperatorSequence{{id_a0, id_a1}, ic});
        EXPECT_EQ(factors_a0a1[0], OperatorSequence({id_a0}, ic));
        EXPECT_EQ(factors_a0a1[1], OperatorSequence({id_a1}, ic));

        // B0B1 should freely factorize
        auto factors_b0b1 = ic.factorize(OperatorSequence{{id_b0, id_b1}, ic});
        EXPECT_EQ(factors_b0b1[0], OperatorSequence({id_b0}, ic));
        EXPECT_EQ(factors_b0b1[1], OperatorSequence({id_b1}, ic));
    }

    TEST(InflationContext, Factorize_W) {
        InflationContext ic{CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}}}, 1};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];
        const auto& obsC = ic.Observables()[2];

        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0});
        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0, 0});
        const auto& obsC_V0 = obsC.variant(std::vector<oper_name_t>{0});

        auto id_a = obsA_V0.operator_offset;
        auto id_b = obsB_V0.operator_offset;
        auto id_c = obsC_V0.operator_offset;

        // AB should not factorize, due to common source
        auto factors_ab = ic.factorize(OperatorSequence{{id_a, id_b}, ic});
        ASSERT_EQ(factors_ab.size(), 1);
        EXPECT_EQ(factors_ab[0], OperatorSequence({id_a, id_b}, ic));

        // BC should not factorize, due to common source
        auto factors_bc = ic.factorize(OperatorSequence{{id_b, id_c}, ic});
        ASSERT_EQ(factors_bc.size(), 1);
        EXPECT_EQ(factors_bc[0], OperatorSequence({id_b, id_c}, ic));

        // AC /can/ factorize when on their own
        auto factors_ac = ic.factorize(OperatorSequence{{id_a, id_c}, ic});
        ASSERT_EQ(factors_ac.size(), 2);
        EXPECT_EQ(factors_ac[0], OperatorSequence({id_a}, ic));
        EXPECT_EQ(factors_ac[1], OperatorSequence({id_c}, ic));

        // ABC does not factorize (conditional mutual info of B!)
        auto factors_abc = ic.factorize(OperatorSequence{{id_a, id_b, id_c}, ic});
        ASSERT_EQ(factors_abc.size(), 1);
        EXPECT_EQ(factors_abc[0], OperatorSequence({id_a, id_b, id_c}, ic));
    }

    TEST(InflationContext, Factorize_Triangle) {
        InflationContext ic{CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}}, 2};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];
        const auto& obsC = ic.Observables()[2];

        const auto id_a00 = obsA.variant(std::vector<oper_name_t>{0, 0}).operator_offset;
        const auto id_a01 = obsA.variant(std::vector<oper_name_t>{0, 1}).operator_offset;
        const auto id_a10 = obsA.variant(std::vector<oper_name_t>{1, 0}).operator_offset;
        const auto id_a11 = obsA.variant(std::vector<oper_name_t>{1, 1}).operator_offset;

        const auto id_b00 = obsB.variant(std::vector<oper_name_t>{0, 0}).operator_offset;
        const auto id_b01 = obsB.variant(std::vector<oper_name_t>{0, 1}).operator_offset;
        const auto id_b10 = obsB.variant(std::vector<oper_name_t>{1, 0}).operator_offset;
        const auto id_b11 = obsB.variant(std::vector<oper_name_t>{1, 1}).operator_offset;

        const auto id_c00 = obsC.variant(std::vector<oper_name_t>{0, 0}).operator_offset;
        const auto id_c01 = obsC.variant(std::vector<oper_name_t>{0, 1}).operator_offset;
        const auto id_c10 = obsC.variant(std::vector<oper_name_t>{1, 0}).operator_offset;
        const auto id_c11 = obsC.variant(std::vector<oper_name_t>{1, 1}).operator_offset;

        // A with itself
        expect_doesnt_factorize(ic, {id_a00, id_a01});
        expect_doesnt_factorize(ic, {id_a00, id_a10});
        expect_factorizes(ic, {id_a00, id_a11});
        expect_factorizes(ic, {id_a01, id_a10});
        expect_doesnt_factorize(ic, {id_a01, id_a11});
        expect_doesnt_factorize(ic, {id_a10, id_a11});
        
        // B with itself
        expect_doesnt_factorize(ic, {id_b00, id_b01});
        expect_doesnt_factorize(ic, {id_b00, id_b10});
        expect_factorizes(ic, {id_b00, id_b11});
        expect_factorizes(ic, {id_b01, id_b10});
        expect_doesnt_factorize(ic, {id_b01, id_b11});
        expect_doesnt_factorize(ic, {id_b10, id_b11});
        
        // C with itself
        expect_doesnt_factorize(ic, {id_c00, id_c01});
        expect_doesnt_factorize(ic, {id_c00, id_c10});
        expect_factorizes(ic, {id_c00, id_c11});
        expect_factorizes(ic, {id_c01, id_c10});
        expect_doesnt_factorize(ic, {id_c01, id_c11});
        expect_doesnt_factorize(ic, {id_c10, id_c11});

        // A with B;  shared index is 1st of A, 1st of B
        expect_doesnt_factorize(ic, {id_a00, id_b00});
        expect_factorizes(ic, {id_a00, id_b10});
        expect_doesnt_factorize(ic, {id_a00, id_b01});
        expect_factorizes(ic, {id_a00, id_b11});
        expect_doesnt_factorize(ic, {id_a01, id_b00});
        expect_factorizes(ic, {id_a01, id_b10});
        expect_doesnt_factorize(ic, {id_a01, id_b01});
        expect_factorizes(ic, {id_a01, id_b11});
        expect_factorizes(ic, {id_a10, id_b00});
        expect_doesnt_factorize(ic, {id_a10, id_b10});
        expect_factorizes(ic, {id_a10, id_b01});
        expect_doesnt_factorize(ic, {id_a10, id_b11});
        expect_factorizes(ic, {id_a11, id_b00});
        expect_doesnt_factorize(ic, {id_a11, id_b10});
        expect_factorizes(ic, {id_a11, id_b01});
        expect_doesnt_factorize(ic, {id_a11, id_b11}); 
        
        // A with C;  shared index is 2nd of A, 2nd of B
        expect_doesnt_factorize(ic, {id_a00, id_c00});
        expect_doesnt_factorize(ic, {id_a00, id_c10});
        expect_factorizes(ic, {id_a00, id_c01});
        expect_factorizes(ic, {id_a00, id_c11});
        expect_factorizes(ic, {id_a01, id_c00});
        expect_factorizes(ic, {id_a01, id_c10});
        expect_doesnt_factorize(ic, {id_a01, id_c01});
        expect_doesnt_factorize(ic, {id_a01, id_c11});
        expect_doesnt_factorize(ic, {id_a10, id_c00});
        expect_doesnt_factorize(ic, {id_a10, id_c10});
        expect_factorizes(ic, {id_a10, id_c01});
        expect_factorizes(ic, {id_a10, id_c11});
        expect_factorizes(ic, {id_a11, id_c00});
        expect_factorizes(ic, {id_a11, id_c10});
        expect_doesnt_factorize(ic, {id_a11, id_c01});
        expect_doesnt_factorize(ic, {id_a11, id_c11});

        // B with C;  shared index is 2nd of B, 1st of C
        expect_doesnt_factorize(ic, {id_b00, id_c00});
        expect_factorizes(ic, {id_b00, id_c10});
        expect_doesnt_factorize(ic, {id_b00, id_c01});
        expect_factorizes(ic, {id_b00, id_c11});
        expect_factorizes(ic, {id_b01, id_c00});
        expect_doesnt_factorize(ic, {id_b01, id_c10});
        expect_factorizes(ic, {id_b01, id_c01});
        expect_doesnt_factorize(ic, {id_b01, id_c11});
        expect_doesnt_factorize(ic, {id_b10, id_c00});
        expect_factorizes(ic, {id_b10, id_c10});
        expect_doesnt_factorize(ic, {id_b10, id_c01});
        expect_factorizes(ic, {id_b10, id_c11});
        expect_factorizes(ic, {id_b11, id_c00});
        expect_doesnt_factorize(ic, {id_b11, id_c10});
        expect_factorizes(ic, {id_b11, id_c01});
        expect_doesnt_factorize(ic, {id_b11, id_c11});
    }

}
