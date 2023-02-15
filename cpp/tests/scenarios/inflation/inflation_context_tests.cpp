/**
 * inflation_context_tests.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/inflation/inflation_context.h"

namespace Moment::Tests {
    using namespace Moment::Inflation;

    namespace {
        void expect_factorizes(const InflationContext& ic, sequence_storage_t&& sequence) {
            OperatorSequence seq{std::move(sequence), ic};
            const size_t size = seq.size();
            auto factors = ic.factorize(seq);
            ASSERT_EQ(factors.size(), size) << "seq = " << seq;
            for (size_t i = 0; i < size; ++i) {
                EXPECT_EQ(factors[i], OperatorSequence({seq[i]}, ic)) << "seq = " << seq;
            }
        }

        void expect_doesnt_factorize(const InflationContext& ic, sequence_storage_t&& sequence) {
            OperatorSequence seq{std::move(sequence), ic};
            const size_t size = seq.size();
            auto factors = ic.factorize(seq);
            ASSERT_EQ(factors.size(), 1) << "seq = " << seq;
            EXPECT_EQ(factors[0], seq);
        }
    }

    TEST(Scenarios_Inflation_InflationContext, Construct_Empty) {
        InflationContext ic{CausalNetwork{{}, {}}, 1};
        EXPECT_EQ(ic.size(), 0);
    }

    TEST(Scenarios_Inflation_InflationContext, Construct_Pair) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 1};
        ASSERT_EQ(ic.size(), 3);
        EXPECT_EQ(ic.source_variant_count(), 1);
        EXPECT_EQ(ic.observable_variant_count(), 2);


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


        // Check XX is simplified
        OperatorSequence xx{{0, 0}, ic};
        ASSERT_EQ(xx.size(), 1);
        EXPECT_EQ(xx[0], 0);

        OperatorSequence yy{{1, 1}, ic};
        ASSERT_EQ(yy.size(), 1);
        EXPECT_EQ(yy[0], 1);
    }

    TEST(Scenarios_Inflation_InflationContext, Construct_CVPair) {
        InflationContext ic{CausalNetwork{{0, 0}, {{0, 1}}}, 1};
        ASSERT_EQ(ic.size(), 2); // X, Y
        EXPECT_EQ(ic.source_variant_count(), 1);
        EXPECT_EQ(ic.observable_variant_count(), 2);

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        EXPECT_EQ(observables[0].id, 0);
        EXPECT_EQ(observables[0].outcomes, 0);
        EXPECT_EQ(observables[0].operators(), 1);
        EXPECT_EQ(observables[0].projective(), false);
        EXPECT_EQ(observables[0].sources.size(), 1);
        EXPECT_TRUE(observables[0].sources.contains(0));

        EXPECT_EQ(observables[1].id, 1);
        EXPECT_EQ(observables[1].outcomes, 0);
        EXPECT_EQ(observables[1].operators(), 1);
        EXPECT_EQ(observables[1].projective(), false);
        EXPECT_EQ(observables[1].sources.size(), 1);
        EXPECT_TRUE(observables[1].sources.contains(0));

        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));

        // Check XX isn't simplified
        OperatorSequence xx{{0, 0}, ic};
        ASSERT_EQ(xx.size(), 2);
        EXPECT_EQ(xx[0], 0);
        EXPECT_EQ(xx[1], 0);

        OperatorSequence yy{{1, 1}, ic};
        ASSERT_EQ(yy.size(), 2);
        EXPECT_EQ(yy[0], 1);
        EXPECT_EQ(yy[1], 1);
    }

    TEST(Scenarios_Inflation_InflationContext, Construct_InflatedCVPair) {
        InflationContext ic{CausalNetwork{{0, 0}, {{0, 1}}}, 2};
        ASSERT_EQ(ic.size(), 4); // X0, X1, Y0, Y1
        ASSERT_EQ(ic.source_variant_count(), 2);
        ASSERT_EQ(ic.observable_variant_count(), 4);

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        
        const auto& A = observables[0];
        EXPECT_EQ(A.id, 0);
        EXPECT_EQ(A.outcomes, 0);
        EXPECT_EQ(A.operators(), 1);
        EXPECT_EQ(A.projective(), false);
        EXPECT_EQ(A.sources.size(), 1);
        EXPECT_TRUE(A.sources.contains(0));
        EXPECT_EQ(A.variant_count, 2);
        ASSERT_EQ(A.variants.size(), 2);
        EXPECT_EQ(A.variant_offset, 0);
        const auto& A0 = A.variants[0];
        EXPECT_EQ(A0.flat_index, 0);
        EXPECT_EQ(A0.operator_offset, 0);
        EXPECT_TRUE(A0.connected_sources.test(0));
        EXPECT_FALSE(A0.connected_sources.test(1));


        const auto& A1 = A.variants[1];
        EXPECT_EQ(A1.flat_index, 1);
        EXPECT_EQ(A1.operator_offset, 1);
        EXPECT_FALSE(A1.connected_sources.test(0));
        EXPECT_TRUE(A1.connected_sources.test(1));

        EXPECT_FALSE(A0.independent(A0));
        EXPECT_TRUE(A0.independent(A1));
        EXPECT_FALSE(A1.independent(A1));

        const auto& B = observables[1];
        EXPECT_EQ(B.id, 1);
        EXPECT_EQ(B.outcomes, 0);
        EXPECT_EQ(B.operators(), 1);
        EXPECT_EQ(B.projective(), false);
        EXPECT_EQ(B.sources.size(), 1);
        EXPECT_TRUE(B.sources.contains(0));
        EXPECT_EQ(B.variant_count, 2);
        ASSERT_EQ(B.variants.size(), 2);
        EXPECT_EQ(B.variant_offset, 2);
        const auto& B0 = B.variants[0];
        EXPECT_EQ(B0.flat_index, 0);
        EXPECT_EQ(B0.operator_offset, 2);
        EXPECT_TRUE(B0.connected_sources.test(0));
        EXPECT_FALSE(B0.connected_sources.test(1));

        const auto& B1 = B.variants[1];
        EXPECT_EQ(B1.flat_index, 1);
        EXPECT_EQ(B1.operator_offset, 3);
        EXPECT_FALSE(B1.connected_sources.test(0));
        EXPECT_TRUE(B1.connected_sources.test(1));


        EXPECT_FALSE(B0.independent(B0));
        EXPECT_TRUE(B0.independent(B1));
        EXPECT_FALSE(B1.independent(B1));


        EXPECT_FALSE(A0.independent(B0));
        EXPECT_TRUE(A0.independent(B1));
        EXPECT_FALSE(A1.independent(B1));
        EXPECT_TRUE(A1.independent(B0));
        EXPECT_FALSE(B0.independent(A0));
        EXPECT_TRUE(B0.independent(A1));
        EXPECT_FALSE(B1.independent(A1));
        EXPECT_TRUE(B1.independent(A0));


        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 1);
        EXPECT_EQ(sources[0].id, 0);
        EXPECT_EQ(sources[0].observables.size(), 2);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].observables.contains(1));
    }


    TEST(Scenarios_Inflation_InflationContext, Construct_UnlinkedCVPair) {
        InflationContext ic{CausalNetwork{{0, 0}, {}}, 2};
        ASSERT_EQ(ic.size(), 2); // X, Y
        EXPECT_EQ(ic.source_variant_count(), 2); // two, implicit sources, not inflated
        EXPECT_EQ(ic.observable_variant_count(), 2); // two observables

        const auto& observables = ic.Observables();
        ASSERT_EQ(observables.size(), 2);
        const auto& A = observables[0];
        EXPECT_EQ(A.id, 0);
        EXPECT_EQ(A.outcomes, 0);
        EXPECT_EQ(A.operators(), 1);
        EXPECT_EQ(A.projective(), false);
        ASSERT_EQ(A.sources.size(), 1);
        EXPECT_TRUE(A.sources.contains(0));
        EXPECT_EQ(A.variant_count, 1);
        ASSERT_EQ(A.variants.size(), 1);
        const auto& A0 = A.variants[0];
        EXPECT_EQ(A0.source_variants.size(), 1);
        
        const auto& B = observables[1];
        EXPECT_EQ(B.id, 1);
        EXPECT_EQ(B.outcomes, 0);
        EXPECT_EQ(B.operators(), 1);
        EXPECT_EQ(B.projective(), false);
        ASSERT_EQ(B.sources.size(), 1);
        EXPECT_TRUE(B.sources.contains(1));
        EXPECT_EQ(B.variant_count, 1);
        ASSERT_EQ(B.variants.size(), 1);
        const auto& B0 = B.variants[0];
        EXPECT_EQ(B0.source_variants.size(), 1);

        // Check (in)dependence:
        EXPECT_FALSE(A0.independent(A0));
        EXPECT_TRUE(A0.independent(B0));
        EXPECT_FALSE(B0.independent(B0));


        const auto& sources = ic.Sources();
        ASSERT_EQ(sources.size(), 2);
        EXPECT_EQ(sources[0].id, 0);
        ASSERT_EQ(sources[0].observables.size(), 1);
        EXPECT_TRUE(sources[0].observables.contains(0));
        EXPECT_TRUE(sources[0].implicit);

        EXPECT_EQ(sources[1].id, 1);
        ASSERT_EQ(sources[1].observables.size(), 1);
        EXPECT_TRUE(sources[1].observables.contains(1));
        EXPECT_TRUE(sources[1].implicit);

        // Check XX isn't simplified
        OperatorSequence xx{{0, 0}, ic};
        ASSERT_EQ(xx.size(), 2);
        EXPECT_EQ(xx[0], 0);
        EXPECT_EQ(xx[1], 0);

        OperatorSequence yy{{1, 1}, ic};
        ASSERT_EQ(yy.size(), 2);
        EXPECT_EQ(yy[0], 1);
        EXPECT_EQ(yy[1], 1);
    }

    TEST(Scenarios_Inflation_InflationContext, NumberOperators) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 2};
        ASSERT_EQ(ic.size(), 6);
        ASSERT_EQ(ic.Observables()[0].count_copies(2), 2);
        ASSERT_EQ(ic.Observables()[1].count_copies(2), 2);
        ASSERT_EQ(ic.Observables()[0].count_operators(2), 4);
        ASSERT_EQ(ic.Observables()[1].count_operators(2), 2);
        EXPECT_EQ(ic.source_variant_count(), 2);
        EXPECT_EQ(ic.observable_variant_count(), 4);

        auto A0_0 = ic.operator_number(0, 0, 0);
        auto A0_1 = ic.operator_number(0, 0, 1);
        auto A1_0 = ic.operator_number(0, 1, 0);
        auto A1_1 = ic.operator_number(0, 1, 1);
        auto B_0 = ic.operator_number(1, 0, 0);
        auto B_1 = ic.operator_number(1, 0, 1);
        std::set<oper_name_t> found_opers = {A0_0, A0_1, A1_0, A1_1, B_0, B_1};
        ASSERT_EQ(found_opers.size(), 6);
    }

    TEST(Scenarios_Inflation_InflationContext, Sequence_Commute) {
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

    TEST(Scenarios_Inflation_InflationContext, Sequence_Orthogonal) {
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

    TEST(Scenarios_Inflation_InflationContext, Sequence_Projector) {
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

    TEST(Scenarios_Inflation_InflationContext, ObservableVariants_Pair) {
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

    TEST(Scenarios_Inflation_InflationContext, ObservableVariants_Triangle) {
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

    TEST(Scenarios_Inflation_InflationContext, VariantIndexing_Triangle) {
        InflationContext ic{CausalNetwork{{2, 2, 2}, {{0, 1}, {1, 2}, {0, 2}}}, 2};

        ASSERT_EQ(ic.observable_variant_count(), 12); // A00, A01, ... B00, ...
        for (oper_name_t index = 0; index < 12; ++index) {
            const auto [obs, var] = ic.index_to_obs_variant(index);
            EXPECT_EQ(obs, index / 4);
            EXPECT_EQ(var, index % 4);
            const auto re_index = ic.obs_variant_to_index(obs, var);
            EXPECT_EQ(re_index, index);
        }
    }

    TEST(Scenarios_Inflation_InflationContext, ObservableIndependence_Pair) {
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

    TEST(Scenarios_Inflation_InflationContext, ObservableIndependence_Triangle) {
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


    TEST(Scenarios_Inflation_InflationContext, Factorize_Pair) {
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

    TEST(Scenarios_Inflation_InflationContext, Factorize_CVPair) {
        InflationContext ic{CausalNetwork{{0, 0}, {{0, 1}}}, 1};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];

        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0});
        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0});

        const auto id_a0 = obsA_V0.operator_offset;
        const auto id_b0 = obsB_V0.operator_offset;
        const std::set all_ids{id_a0, id_b0};
        ASSERT_EQ(all_ids.size(), 2);

        // 0, I, a0, and b0 should all just pass through unfactorized
        auto factors_0 = ic.factorize(OperatorSequence::Zero(ic));
        ASSERT_EQ(factors_0.size(), 1);
        EXPECT_EQ(factors_0[0], OperatorSequence::Zero(ic));

        auto factors_I = ic.factorize(OperatorSequence::Identity(ic));
        ASSERT_EQ(factors_I.size(), 1);
        EXPECT_EQ(factors_I[0], OperatorSequence::Identity(ic));

        auto factors_a0 = ic.factorize(OperatorSequence{{id_a0}, ic});
        ASSERT_EQ(factors_a0.size(), 1);
        EXPECT_EQ(factors_a0[0], OperatorSequence({id_a0}, ic));

        auto factors_b0 = ic.factorize(OperatorSequence{{id_b0}, ic});
        ASSERT_EQ(factors_b0.size(), 1);
        EXPECT_EQ(factors_b0[0], OperatorSequence({id_b0}, ic));

        // A0B0 should not factorize, due to common source
        auto factors_a0b0 = ic.factorize(OperatorSequence{{id_a0, id_b0}, ic});
        ASSERT_EQ(factors_a0b0.size(), 1);
        EXPECT_EQ(factors_a0b0[0], OperatorSequence({id_a0, id_b0}, ic));

        // A0A0 should not factorize, due to common source
        auto factors_a0a0 = ic.factorize(OperatorSequence{{id_a0, id_a0}, ic});
        ASSERT_EQ(factors_a0a0.size(), 1);
        EXPECT_EQ(factors_a0a0[0], OperatorSequence({id_a0, id_a0}, ic));

        // B0B0 should not factorize, due to common source
        auto factors_b0b0 = ic.factorize(OperatorSequence{{id_b0, id_b0}, ic});
        ASSERT_EQ(factors_b0b0.size(), 1);
        EXPECT_EQ(factors_b0b0[0], OperatorSequence({id_b0, id_b0}, ic));
    }

    TEST(Scenarios_Inflation_InflationContext, Factorize_CVUnlinkedPair) {
        InflationContext ic{CausalNetwork{{0, 0}, {}}, 1};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];

        const auto& obsA_V0 = obsA.variants[0];
        const auto& obsB_V0 = obsB.variants[0];

        const auto id_a0 = obsA_V0.operator_offset;
        const auto id_b0 = obsB_V0.operator_offset;
        const std::set all_ids{id_a0, id_b0};
        ASSERT_EQ(all_ids.size(), 2);

        // 0, I, a0, and b0 should all just pass through unfactorized
        auto factors_0 = ic.factorize(OperatorSequence::Zero(ic));
        ASSERT_EQ(factors_0.size(), 1);
        EXPECT_EQ(factors_0[0], OperatorSequence::Zero(ic));

        auto factors_I = ic.factorize(OperatorSequence::Identity(ic));
        ASSERT_EQ(factors_I.size(), 1);
        EXPECT_EQ(factors_I[0], OperatorSequence::Identity(ic));

        auto factors_a0 = ic.factorize(OperatorSequence{{id_a0}, ic});
        ASSERT_EQ(factors_a0.size(), 1);
        EXPECT_EQ(factors_a0[0], OperatorSequence({id_a0}, ic));

        auto factors_b0 = ic.factorize(OperatorSequence{{id_b0}, ic});
        ASSERT_EQ(factors_b0.size(), 1);
        EXPECT_EQ(factors_b0[0], OperatorSequence({id_b0}, ic));

        // A0B0 should factorize
        auto factors_a0b0 = ic.factorize(OperatorSequence{{id_a0, id_b0}, ic});
        ASSERT_EQ(factors_a0b0.size(), 2);
        EXPECT_EQ(factors_a0b0[0], OperatorSequence({id_a0}, ic));
        EXPECT_EQ(factors_a0b0[1], OperatorSequence({id_b0}, ic));

        // A0A0 should not factorize, due to common source
        auto factors_a0a0 = ic.factorize(OperatorSequence{{id_a0, id_a0}, ic});
        ASSERT_EQ(factors_a0a0.size(), 1);
        EXPECT_EQ(factors_a0a0[0], OperatorSequence({id_a0, id_a0}, ic));

        // B0B0 should not factorize, due to common source
        auto factors_b0b0 = ic.factorize(OperatorSequence{{id_b0, id_b0}, ic});
        ASSERT_EQ(factors_b0b0.size(), 1);
        EXPECT_EQ(factors_b0b0[0], OperatorSequence({id_b0, id_b0}, ic));
    }

    TEST(Scenarios_Inflation_InflationContext, Factorize_PairSingleton) {
        InflationContext ic{CausalNetwork{{2, 2, 0}, {{0,1}}}, 1};
        ASSERT_EQ(ic.Observables().size(), 3);
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];
        const auto& obsC = ic.Observables()[2];

        const auto& obsA_V0 = obsA.variants[0];
        const auto& obsB_V0 = obsB.variants[0];
        const auto& obsC_V0 = obsC.variants[0];

        const auto id_a0 = obsA_V0.operator_offset;
        const auto id_b0 = obsB_V0.operator_offset;
        const auto id_c0 = obsC_V0.operator_offset;
        const std::set all_ids{id_a0, id_b0, id_c0};
        ASSERT_EQ(all_ids.size(), 3);

        // 0, I, a0, b0 and c0 should all just pass through unfactorized
        auto factors_0 = ic.factorize(OperatorSequence::Zero(ic));
        ASSERT_EQ(factors_0.size(), 1);
        EXPECT_EQ(factors_0[0], OperatorSequence::Zero(ic));

        auto factors_I = ic.factorize(OperatorSequence::Identity(ic));
        ASSERT_EQ(factors_I.size(), 1);
        EXPECT_EQ(factors_I[0], OperatorSequence::Identity(ic));

        auto factors_a0 = ic.factorize(OperatorSequence{{id_a0}, ic});
        ASSERT_EQ(factors_a0.size(), 1);
        EXPECT_EQ(factors_a0[0], OperatorSequence({id_a0}, ic));

        auto factors_b0 = ic.factorize(OperatorSequence{{id_b0}, ic});
        ASSERT_EQ(factors_b0.size(), 1);
        EXPECT_EQ(factors_b0[0], OperatorSequence({id_b0}, ic));

        auto factors_c0 = ic.factorize(OperatorSequence{{id_c0}, ic});
        ASSERT_EQ(factors_c0.size(), 1);
        EXPECT_EQ(factors_c0[0], OperatorSequence({id_c0}, ic));

        // A0B0 shouldn't factorize, due to common source
        auto factors_a0b0 = ic.factorize(OperatorSequence{{id_a0, id_b0}, ic});
        ASSERT_EQ(factors_a0b0.size(), 1);
        EXPECT_EQ(factors_a0b0[0], OperatorSequence({id_a0, id_b0}, ic));

        // A0A0 should not factorize, same object [moreover, a0^2 = a0]
        auto factors_a0a0 = ic.factorize(OperatorSequence{{id_a0, id_a0}, ic});
        ASSERT_EQ(factors_a0a0.size(), 1);
        EXPECT_EQ(factors_a0a0[0], OperatorSequence({id_a0, id_a0}, ic));

        // B0B0 should not factorize, same object [moreover, b0^2 = b0]
        auto factors_b0b0 = ic.factorize(OperatorSequence{{id_b0, id_b0}, ic});
        ASSERT_EQ(factors_b0b0.size(), 1);
        EXPECT_EQ(factors_b0b0[0], OperatorSequence({id_b0, id_b0}, ic));

        // C0C0 should not factorize, same object
        auto factors_c0c0 = ic.factorize(OperatorSequence{{id_c0, id_c0}, ic});
        ASSERT_EQ(factors_c0c0.size(), 1);
        EXPECT_EQ(factors_c0c0[0], OperatorSequence({id_c0, id_c0}, ic));
        
        // A0C0 should factorize
        auto factors_a0c0 = ic.factorize(OperatorSequence{{id_a0, id_c0}, ic});
        ASSERT_EQ(factors_a0c0.size(), 2);
        EXPECT_EQ(factors_a0c0[0], OperatorSequence({id_a0}, ic));
        EXPECT_EQ(factors_a0c0[1], OperatorSequence({id_c0}, ic));
        
        // B0C0 should factorize
        auto factors_b0c0 = ic.factorize(OperatorSequence{{id_b0, id_c0}, ic});
        ASSERT_EQ(factors_b0c0.size(), 2);
        EXPECT_EQ(factors_b0c0[0], OperatorSequence({id_b0}, ic));
        EXPECT_EQ(factors_b0c0[1], OperatorSequence({id_c0}, ic));

    }

    TEST(Scenarios_Inflation_InflationContext, Factorize_InflatedCVPair) {
        InflationContext ic{CausalNetwork{{0, 0}, {{0, 1}}}, 2};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];

        const auto& obsA_V0 = obsA.variant(std::vector<oper_name_t>{0});
        const auto& obsA_V1 = obsA.variant(std::vector<oper_name_t>{1});
        const auto& obsB_V0 = obsB.variant(std::vector<oper_name_t>{0});
        const auto& obsB_V1 = obsB.variant(std::vector<oper_name_t>{1});

        const auto id_a0 = obsA_V0.operator_offset;
        const auto id_a1 = obsA_V1.operator_offset;
        const auto id_b0 = obsB_V0.operator_offset;
        const auto id_b1 = obsB_V1.operator_offset;
        const std::set all_ids{id_a0, id_a1, id_b0, id_b1};
        ASSERT_EQ(all_ids.size(), 4);

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

        // A0A0 should not factorize, due to common source
        auto factors_a0a0 = ic.factorize(OperatorSequence{{id_a0, id_a0}, ic});
        ASSERT_EQ(factors_a0a0.size(), 1);
        EXPECT_EQ(factors_a0a0[0], OperatorSequence({id_a0, id_a0}, ic));

        // A1A1 should not factorize, due to common source
        auto factors_a1a1 = ic.factorize(OperatorSequence{{id_a1, id_a1}, ic});
        ASSERT_EQ(factors_a1a1.size(), 1);
        EXPECT_EQ(factors_a1a1[0], OperatorSequence({id_a1, id_a1}, ic));

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

        // B0B0 should not factorize, due to common source
        auto factors_b0b0 = ic.factorize(OperatorSequence{{id_b0, id_b0}, ic});
        ASSERT_EQ(factors_b0b0.size(), 1);
        EXPECT_EQ(factors_b0b0[0], OperatorSequence({id_b0, id_b0}, ic));

        // B1B1 should not factorize, due to common source
        auto factors_b1b1 = ic.factorize(OperatorSequence{{id_b1, id_b1}, ic});
        ASSERT_EQ(factors_b1b1.size(), 1);
        EXPECT_EQ(factors_b1b1[0], OperatorSequence({id_b1, id_b1}, ic));

        // B0B1 should freely factorize
        auto factors_b0b1 = ic.factorize(OperatorSequence{{id_b0, id_b1}, ic});
        EXPECT_EQ(factors_b0b1[0], OperatorSequence({id_b0}, ic));
        EXPECT_EQ(factors_b0b1[1], OperatorSequence({id_b1}, ic));
    }

    TEST(Scenarios_Inflation_InflationContext, Factorize_W) {
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

    TEST(Scenarios_Inflation_InflationContext, Factorize_Triangle) {
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

    TEST(Scenarios_Inflation_InflationContext, CanonicalMoment_Pair) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0, 1}}}, 2};
        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];

        const auto& obsA0 = obsA.variants[0];
        const auto& obsA1 = obsA.variants[1];
        const auto& obsB0 = obsB.variants[0];
        const auto& obsB1 = obsB.variants[1];

        const oper_name_t a0_0 = obsA0.operator_offset;
        const oper_name_t a0_1 = obsA0.operator_offset + 1;
        const oper_name_t a1_0 = obsA1.operator_offset;
        const oper_name_t a1_1 = obsA1.operator_offset + 1;
        const oper_name_t b0 = obsB0.operator_offset;
        const oper_name_t b1 = obsB1.operator_offset;

        const std::set<oper_name_t> all_elems{a0_0, a0_1, a1_0, a1_1, b0, b1};
        ASSERT_EQ(all_elems.size(), 6);

        // First outcome A
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_0}, ic}), OperatorSequence({a0_0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_1}, ic}), OperatorSequence({a0_1}, ic));

        // Second outcome A
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a1_0}, ic}), OperatorSequence({a0_0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a1_1}, ic}), OperatorSequence({a0_1}, ic));

        // Outcome B
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{b0}, ic}), OperatorSequence({b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{b1}, ic}), OperatorSequence({b0}, ic));

        // a0_0 b0 -> a0_0 b0; but same for a1_0 b_0;
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_0, b0}, ic}), OperatorSequence({a0_0, b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a1_0, b1}, ic}), OperatorSequence({a0_0, b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_1, b0}, ic}), OperatorSequence({a0_1, b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a1_1, b1}, ic}), OperatorSequence({a0_1, b0}, ic));

        // a0_0 a1_0, cannot further simplify (but could factor then simplify)
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_0, a1_0}, ic}), OperatorSequence({a0_0, a1_0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_0, a1_1}, ic}), OperatorSequence({a0_0, a1_1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_1, a1_0}, ic}), OperatorSequence({a0_1, a1_0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_1, a1_1}, ic}), OperatorSequence({a0_1, a1_1}, ic));

        // a0_0 b1 -> a0_0 b1; but a1_0 b0 -> a0_0 b1 too??
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_0, b1}, ic}), OperatorSequence({a0_0, b1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a1_0, b0}, ic}), OperatorSequence({a0_0, b1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a0_1, b1}, ic}), OperatorSequence({a0_1, b1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a1_1, b0}, ic}), OperatorSequence({a0_1, b1}, ic));
    }

    TEST(Scenarios_Inflation_InflationContext, CanonicalMoment_TwoSourceTwoObs) {
        InflationContext ic{CausalNetwork{{2, 2}, {{0}, {0, 1}}}, 2};

        const auto& obsA = ic.Observables()[0];
        const auto& obsB = ic.Observables()[1];
        const auto& obsA00 = obsA.variant(std::vector<oper_name_t>{0, 0});
        const auto& obsA01 = obsA.variant(std::vector<oper_name_t>{0, 1});
        const auto& obsA10 = obsA.variant(std::vector<oper_name_t>{1, 0});
        const auto& obsA11 = obsA.variant(std::vector<oper_name_t>{1, 1});

        const auto& obsB0 = obsB.variants[0];
        const auto& obsB1 = obsB.variants[1];

        const oper_name_t a00 = obsA00.operator_offset;
        const oper_name_t a01 = obsA01.operator_offset;
        const oper_name_t a10 = obsA10.operator_offset;
        const oper_name_t a11 = obsA11.operator_offset;
        const oper_name_t b0 = obsB0.operator_offset;
        const oper_name_t b1 = obsB1.operator_offset;

        const std::set<oper_name_t> all_elems{a00, a01, a10, a11, b0, b1};
        ASSERT_EQ(all_elems.size(), 6);

        // Outcome A
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a00}, ic}), OperatorSequence({a00}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a01}, ic}), OperatorSequence({a00}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a10}, ic}), OperatorSequence({a00}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a11}, ic}), OperatorSequence({a00}, ic));

        // Outcome B
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{b0}, ic}), OperatorSequence({b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{b1}, ic}), OperatorSequence({b0}, ic));

        // Linked AB
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a00, b0}, ic}), OperatorSequence({a00, b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a10, b0}, ic}), OperatorSequence({a00, b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a01, b1}, ic}), OperatorSequence({a00, b0}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a11, b1}, ic}), OperatorSequence({a00, b0}, ic));

        // Unlinked AB
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a00, b1}, ic}), OperatorSequence({a00, b1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a10, b1}, ic}), OperatorSequence({a00, b1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a01, b0}, ic}), OperatorSequence({a00, b1}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a11, b0}, ic}), OperatorSequence({a00, b1}, ic));

        // A with itself [should factorize anyway]
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a00, a11}, ic}), OperatorSequence({a00, a11}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a11, a00}, ic}), OperatorSequence({a00, a11}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a01, a10}, ic}), OperatorSequence({a00, a11}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a10, a01}, ic}), OperatorSequence({a00, a11}, ic));

        // A with itself [does not factorize]
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a00, a01}, ic}), OperatorSequence({a00, a01}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a10, a11}, ic}), OperatorSequence({a00, a01}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a01, a11}, ic}), OperatorSequence({a00, a10}, ic));
        EXPECT_EQ(ic.canonical_moment(OperatorSequence{{a10, a00}, ic}), OperatorSequence({a00, a10}, ic));
    }

    TEST(Scenarios_Inflation_InflationContext, CanonicalVariants_TwoSourceTwoObs) {
        InflationContext ic{CausalNetwork{{2, 2}, {{0}, {0, 1}}}, 2}; // A00,A01,A10,A11,B0,B1

        ASSERT_EQ(ic.observable_variant_count(), 6);

        for (oper_name_t a_var = 0; a_var < 4; ++a_var) {
            auto canA = ic.canonical_variants({{0LL, a_var}});
            ASSERT_EQ(canA.size(), 1);
            EXPECT_EQ(canA[0], OVIndex(0LL, 0LL));
        }

        for (oper_name_t b_var = 0; b_var < 2; ++b_var) {
            auto canB = ic.canonical_variants({{1LL, b_var}});
            ASSERT_EQ(canB.size(), 1);
            EXPECT_EQ(canB[0], OVIndex(1LL, 0LL));
        }

        auto fromA00B0 = ic.canonical_variants({{0LL, 0LL}, {1LL, 0LL}}); // A00 B0
        ASSERT_EQ(fromA00B0.size(), 2);
        EXPECT_EQ(fromA00B0[0], OVIndex(0LL, 0LL));
        EXPECT_EQ(fromA00B0[1], OVIndex(1LL, 0LL));

        auto fromA01B1 = ic.canonical_variants({{0LL, 1LL}, {1LL, 1LL}}); // A01 B1
        ASSERT_EQ(fromA01B1.size(), 2);
        EXPECT_EQ(fromA01B1[0], OVIndex(0LL, 0LL));
        EXPECT_EQ(fromA01B1[1], OVIndex(1LL, 0LL));

        auto fromB1A01 = ic.canonical_variants({{1LL, 1LL}, {0LL, 1LL}}); // B1 A01
        ASSERT_EQ(fromB1A01.size(), 2);
        EXPECT_EQ(fromB1A01[0], OVIndex(0LL, 0LL));
        EXPECT_EQ(fromB1A01[1], OVIndex(1LL, 0LL));
    }


    TEST(Scenarios_Inflation_InflationContext, UnflattenOutcomeIndex) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0}, {0, 1}}}, 2}; // mmts: A00, A01, A10, A11, B0, B1
        ASSERT_EQ(ic.observable_variant_count(), 6);

        const std::vector<OVIndex> ov{{0, 0}, {1, 0}}; // should have 6 outcomes...

        // 0,0
        auto ovo_00 = ic.unflatten_outcome_index(ov, 0);
        ASSERT_EQ(ovo_00.size(), 2);
        EXPECT_EQ(ovo_00[0].observable_variant.observable, 0);
        EXPECT_EQ(ovo_00[0].observable_variant.variant, 0);
        EXPECT_EQ(ovo_00[0].outcome, 0);
        EXPECT_EQ(ovo_00[1].observable_variant.observable, 1);
        EXPECT_EQ(ovo_00[1].observable_variant.variant, 0);
        EXPECT_EQ(ovo_00[1].outcome, 0);

        // 0,1
        auto ovo_01 = ic.unflatten_outcome_index(ov, 1);
        ASSERT_EQ(ovo_01.size(), 2);
        EXPECT_EQ(ovo_01[0].observable_variant.observable, 0);
        EXPECT_EQ(ovo_01[0].observable_variant.variant, 0);
        EXPECT_EQ(ovo_01[0].outcome, 0);
        EXPECT_EQ(ovo_01[1].observable_variant.observable, 1);
        EXPECT_EQ(ovo_01[1].observable_variant.variant, 0);
        EXPECT_EQ(ovo_01[1].outcome, 1);  // 0,0
        
        // 1,0
        auto ovo_10 = ic.unflatten_outcome_index(ov, 2);
        ASSERT_EQ(ovo_10.size(), 2);
        EXPECT_EQ(ovo_10[0].observable_variant.observable, 0);
        EXPECT_EQ(ovo_10[0].observable_variant.variant, 0);
        EXPECT_EQ(ovo_10[0].outcome, 1);
        EXPECT_EQ(ovo_10[1].observable_variant.observable, 1);
        EXPECT_EQ(ovo_10[1].observable_variant.variant, 0);
        EXPECT_EQ(ovo_10[1].outcome, 0);

        // 1,1
        auto ovo_11 = ic.unflatten_outcome_index(ov, 3);
        ASSERT_EQ(ovo_11.size(), 2);
        EXPECT_EQ(ovo_11[0].observable_variant.observable, 0);
        EXPECT_EQ(ovo_11[0].observable_variant.variant, 0);
        EXPECT_EQ(ovo_11[0].outcome, 1);
        EXPECT_EQ(ovo_11[1].observable_variant.observable, 1);
        EXPECT_EQ(ovo_11[1].observable_variant.variant, 0);
        EXPECT_EQ(ovo_11[1].outcome, 1);
        
        // 2,0
        auto ovo_20 = ic.unflatten_outcome_index(ov, 4);
        ASSERT_EQ(ovo_20.size(), 2);
        EXPECT_EQ(ovo_20[0].observable_variant.observable, 0);
        EXPECT_EQ(ovo_20[0].observable_variant.variant, 0);
        EXPECT_EQ(ovo_20[0].outcome, 2);
        EXPECT_EQ(ovo_20[1].observable_variant.observable, 1);
        EXPECT_EQ(ovo_20[1].observable_variant.variant, 0);
        EXPECT_EQ(ovo_20[1].outcome, 0);

        // 2,1
        auto ovo_21 = ic.unflatten_outcome_index(ov, 5);
        ASSERT_EQ(ovo_21.size(), 2);
        EXPECT_EQ(ovo_21[0].observable_variant.observable, 0);
        EXPECT_EQ(ovo_21[0].observable_variant.variant, 0);
        EXPECT_EQ(ovo_21[0].outcome, 2);
        EXPECT_EQ(ovo_21[1].observable_variant.observable, 1);
        EXPECT_EQ(ovo_21[1].observable_variant.variant, 0);
        EXPECT_EQ(ovo_21[1].outcome, 1);
    }

    TEST(Scenarios_Inflation_InflationContext, FlattenOutcomeIndex) {
        InflationContext ic{CausalNetwork{{3, 2}, {{0}, {0, 1}}}, 2}; // mmts: A00, A01, A10, A11, B0, B1
        ASSERT_EQ(ic.observable_variant_count(), 6);

        EXPECT_EQ(ic.flatten_outcome_index(std::vector<OVOIndex>{{0, 0, 0}, {1, 0, 0}}), 0);
        EXPECT_EQ(ic.flatten_outcome_index(std::vector<OVOIndex>{{0, 0, 0}, {1, 0, 1}}), 1);
        EXPECT_EQ(ic.flatten_outcome_index(std::vector<OVOIndex>{{0, 0, 1}, {1, 0, 0}}), 2);
        EXPECT_EQ(ic.flatten_outcome_index(std::vector<OVOIndex>{{0, 0, 1}, {1, 0, 1}}), 3);
        EXPECT_EQ(ic.flatten_outcome_index(std::vector<OVOIndex>{{0, 0, 2}, {1, 0, 0}}), 4);
        EXPECT_EQ(ic.flatten_outcome_index(std::vector<OVOIndex>{{0, 0, 2}, {1, 0, 1}}), 5);

    }

}
