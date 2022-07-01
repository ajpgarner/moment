/**
 * collins_gisin_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/moment_matrix.h"
#include "operators/collins_gisin.h"

namespace NPATK::Tests {

    TEST(CollinsGisin, OnePartyOneMeasurementThreeOutcomes) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(1, 1, 3));
        MomentMatrix momentMatrix{contextPtr, 1};
        const auto& alice = contextPtr->Parties[0];
        auto a0_loc = momentMatrix.UniqueSequences.where(
                        OperatorSequence({alice.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(a0_loc, nullptr);
        auto a1_loc = momentMatrix.UniqueSequences.where(
                        OperatorSequence({alice.measurement_outcome(0, 1)}, contextPtr.get()));
        ASSERT_NE(a1_loc, nullptr);
        ASSERT_NE(a0_loc->Id(), a1_loc->Id());

        CollinsGisinForm cgForm{momentMatrix, 1};
        ASSERT_EQ(cgForm.Level, 1);

        std::vector<size_t> empty{};
        auto idSpan = cgForm.get_global(empty);
        ASSERT_EQ(idSpan.size(), 1);
        EXPECT_EQ(idSpan[0], 1);

        std::vector<size_t> a_index{0};
        auto aSpan = cgForm.get_global(a_index);
        ASSERT_EQ(aSpan.size(), 2);
        EXPECT_EQ(aSpan[0], a0_loc->Id());
        EXPECT_EQ(aSpan[1], a1_loc->Id());
    }

    TEST(CollinsGisin, TwoPartyTwoMeasurementTwoOutcomes) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 2, 2));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto& alice = contextPtr->Parties[0];
        const auto& bob = contextPtr->Parties[1];

        MomentMatrix momentMatrix{contextPtr, 1};
        auto alice_a0 = momentMatrix.UniqueSequences.where(
                        OperatorSequence({alice.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0, nullptr);
        auto alice_b0 = momentMatrix.UniqueSequences.where(
                        OperatorSequence({alice.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0, nullptr);
        auto bob_a0 = momentMatrix.UniqueSequences.where(
                        OperatorSequence({bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(bob_a0, nullptr);
        auto bob_b0 = momentMatrix.UniqueSequences.where(
                        OperatorSequence({bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(bob_b0, nullptr);

        auto alice_a0_bob_a0 = momentMatrix.UniqueSequences.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        auto alice_a0_bob_b0 = momentMatrix.UniqueSequences.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        auto alice_b0_bob_a0 = momentMatrix.UniqueSequences.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        auto alice_b0_bob_b0 = momentMatrix.UniqueSequences.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_b0, nullptr);

        CollinsGisinForm cgForm{momentMatrix, 2};
        ASSERT_EQ(cgForm.Level, 2);

        auto id_span = cgForm.get_global({});
        ASSERT_EQ(id_span.size(), 1);
        EXPECT_EQ(id_span[0], 1);

        auto alice_a_span = cgForm.get_global({0});
        ASSERT_EQ(alice_a_span.size(), 1);
        EXPECT_EQ(alice_a_span[0], alice_a0->Id());

        auto alice_b_span = cgForm.get_global({1});
        ASSERT_EQ(alice_b_span.size(), 1);
        EXPECT_EQ(alice_b_span[0], alice_b0->Id());

        auto bob_a_span = cgForm.get_global({2});
        ASSERT_EQ(bob_a_span.size(), 1);
        EXPECT_EQ(bob_a_span[0], bob_a0->Id());

        auto bob_b_span = cgForm.get_global({3});
        ASSERT_EQ(bob_b_span.size(), 1);
        EXPECT_EQ(bob_b_span[0], bob_b0->Id());

        auto aa_span = cgForm.get_global({0, 2});
        ASSERT_EQ(aa_span.size(), 1);
        EXPECT_EQ(aa_span[0], alice_a0_bob_a0->Id());

        auto ab_span = cgForm.get_global({0, 3});
        ASSERT_EQ(ab_span.size(), 1);
        EXPECT_EQ(ab_span[0], alice_a0_bob_b0->Id());

        auto ba_span = cgForm.get_global({1, 2});
        ASSERT_EQ(ba_span.size(), 1);
        EXPECT_EQ(ba_span[0], alice_b0_bob_a0->Id());

        auto bb_span = cgForm.get_global({1, 3});
        ASSERT_EQ(bb_span.size(), 1);
        EXPECT_EQ(bb_span[0], alice_b0_bob_b0->Id());

    }
}