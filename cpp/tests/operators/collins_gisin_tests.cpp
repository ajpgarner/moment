/**
 * collins_gisin_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/moment_matrix.h"
#include "operators/collins_gisin.h"
#include "operators/matrix_system.h"

namespace NPATK::Tests {

    TEST(CollinsGisin, OnePartyOneMeasurementThreeOutcomes) {

        auto contextPtr = std::make_shared<Context>(Party::MakeList(1, 1, 3));
        MatrixSystem system{contextPtr};
        auto& momentMatrix = system.CreateMomentMatrix(1);

        const auto& alice = contextPtr->Parties[0];
        auto a0_loc = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(a0_loc, nullptr);
        auto a1_loc = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(0, 1)}, contextPtr.get()));
        ASSERT_NE(a1_loc, nullptr);
        ASSERT_NE(a0_loc->Id(), a1_loc->Id());

        const CollinsGisinForm& cgForm = momentMatrix.CollinsGisin();
        const CollinsGisinForm& cgForm2 = momentMatrix.CollinsGisin();
        EXPECT_EQ(&cgForm, &cgForm2);
        ASSERT_EQ(cgForm.Level, 1);

        std::vector<size_t> empty{};
        auto idSpan = cgForm.get(empty);
        ASSERT_EQ(idSpan.size(), 1);
        EXPECT_EQ(idSpan[0], 1);

        std::vector<size_t> a_index{0};
        auto aSpan = cgForm.get(a_index);
        ASSERT_EQ(aSpan.size(), 2);
        EXPECT_EQ(aSpan[0], a0_loc->Id());
        EXPECT_EQ(aSpan[1], a1_loc->Id());
    }

    TEST(CollinsGisin, TwoPartyTwoMeasurementTwoOutcomes) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 2, 2));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto& alice = contextPtr->Parties[0];
        const auto& bob = contextPtr->Parties[1];

        MatrixSystem system{contextPtr};
        auto& momentMatrix = system.CreateMomentMatrix(1);

        auto alice_a0 = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0, nullptr);
        auto alice_b0 = momentMatrix.Symbols.where(
                        OperatorSequence({alice.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0, nullptr);
        auto bob_a0 = momentMatrix.Symbols.where(
                        OperatorSequence({bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(bob_a0, nullptr);
        auto bob_b0 = momentMatrix.Symbols.where(
                        OperatorSequence({bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(bob_b0, nullptr);

        auto alice_a0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        auto alice_a0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        auto alice_b0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        auto alice_b0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_b0, nullptr);

        const CollinsGisinForm& cgForm = momentMatrix.CollinsGisin();
        ASSERT_EQ(cgForm.Level, 2);

        auto id_span = cgForm.get({});
        ASSERT_EQ(id_span.size(), 1);
        EXPECT_EQ(id_span[0], 1);

        auto alice_a_span = cgForm.get({0});
        ASSERT_EQ(alice_a_span.size(), 1);
        EXPECT_EQ(alice_a_span[0], alice_a0->Id());

        auto alice_b_span = cgForm.get({1});
        ASSERT_EQ(alice_b_span.size(), 1);
        EXPECT_EQ(alice_b_span[0], alice_b0->Id());

        auto bob_a_span = cgForm.get({2});
        ASSERT_EQ(bob_a_span.size(), 1);
        EXPECT_EQ(bob_a_span[0], bob_a0->Id());

        auto bob_b_span = cgForm.get({3});
        ASSERT_EQ(bob_b_span.size(), 1);
        EXPECT_EQ(bob_b_span[0], bob_b0->Id());

        auto aa_span = cgForm.get({0, 2});
        ASSERT_EQ(aa_span.size(), 1);
        EXPECT_EQ(aa_span[0], alice_a0_bob_a0->Id());

        auto ab_span = cgForm.get({0, 3});
        ASSERT_EQ(ab_span.size(), 1);
        EXPECT_EQ(ab_span[0], alice_a0_bob_b0->Id());

        auto ba_span = cgForm.get({1, 2});
        ASSERT_EQ(ba_span.size(), 1);
        EXPECT_EQ(ba_span[0], alice_b0_bob_a0->Id());

        auto bb_span = cgForm.get({1, 3});
        ASSERT_EQ(bb_span.size(), 1);
        EXPECT_EQ(bb_span[0], alice_b0_bob_b0->Id());
    }

    TEST(CollinsGisin, GetWithFixed222) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 2, 2));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto& alice = contextPtr->Parties[0];
        const auto& bob = contextPtr->Parties[1];

        MatrixSystem system{contextPtr};
        auto& momentMatrix = system.CreateMomentMatrix(1);

        auto alice_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0, nullptr);
        auto alice_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0, nullptr);
        auto bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(bob_a0, nullptr);
        auto bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(bob_b0, nullptr);

        auto alice_a0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        auto alice_a0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        auto alice_b0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        auto alice_b0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_b0, nullptr);

        const CollinsGisinForm& cgForm = momentMatrix.CollinsGisin();
        ASSERT_EQ(cgForm.Level, 2);

        auto fixA0freeB0 = cgForm.get({0, 2}, {0, -1});
        ASSERT_EQ(fixA0freeB0.size(), 1);
        EXPECT_EQ(fixA0freeB0[0], alice_a0_bob_a0->Id());

        auto fixA0freeB1 = cgForm.get({0, 3}, {0, -1});
        ASSERT_EQ(fixA0freeB1.size(), 1);
        EXPECT_EQ(fixA0freeB1[0], alice_a0_bob_b0->Id());

        auto fixB0freeB0 = cgForm.get({1, 2}, {0, -1});
        ASSERT_EQ(fixB0freeB0.size(), 1);
        EXPECT_EQ(fixB0freeB0[0], alice_b0_bob_a0->Id());

        auto fixB0freeB1 = cgForm.get({1, 3}, {0, -1});
        ASSERT_EQ(fixB0freeB1.size(), 1);
        EXPECT_EQ(fixB0freeB1[0], alice_b0_bob_b0->Id());

        auto freeA0fixB0 = cgForm.get({0, 2}, {-1, 0});
        ASSERT_EQ(freeA0fixB0.size(), 1);
        EXPECT_EQ(freeA0fixB0[0], alice_a0_bob_a0->Id());

        auto freeA0fixB1 = cgForm.get({0, 3}, {-1, 0});
        ASSERT_EQ(freeA0fixB1.size(), 1);
        EXPECT_EQ(freeA0fixB1[0], alice_a0_bob_b0->Id());

        auto freeB0fixB0 = cgForm.get({1, 2}, {-1, 0});
        ASSERT_EQ(freeB0fixB0.size(), 1);
        EXPECT_EQ(freeB0fixB0[0], alice_b0_bob_a0->Id());

        auto freeB0fixB1 = cgForm.get({1, 3}, {-1, 0});
        ASSERT_EQ(freeB0fixB1.size(), 1);
        EXPECT_EQ(freeB0fixB1[0], alice_b0_bob_b0->Id());
        
        auto fixA0fixB0 = cgForm.get({0, 2}, {0, 0});
        ASSERT_EQ(fixA0fixB0.size(), 1);
        EXPECT_EQ(fixA0fixB0[0], alice_a0_bob_a0->Id());

        auto fixA0fixB1 = cgForm.get({0, 3}, {0, 0});
        ASSERT_EQ(fixA0fixB1.size(), 1);
        EXPECT_EQ(fixA0fixB1[0], alice_a0_bob_b0->Id());

        auto fixA1fixB0 = cgForm.get({1, 2}, {0, 0});
        ASSERT_EQ(fixA1fixB0.size(), 1);
        EXPECT_EQ(fixA1fixB0[0], alice_b0_bob_a0->Id());

        auto fixA1fixB1 = cgForm.get({1, 3}, {0, 0});
        ASSERT_EQ(fixA1fixB1.size(), 1);
        EXPECT_EQ(fixA1fixB1[0], alice_b0_bob_b0->Id());
        
        auto freeA0freeB0 = cgForm.get({0, 2}, {-1, -1});
        ASSERT_EQ(freeA0freeB0.size(), 1);
        EXPECT_EQ(freeA0freeB0[0], alice_a0_bob_a0->Id());

        auto freeA0freeB1 = cgForm.get({0, 3}, {-1, -1});
        ASSERT_EQ(freeA0freeB1.size(), 1);
        EXPECT_EQ(freeA0freeB1[0], alice_a0_bob_b0->Id());

        auto freeA1freeB0 = cgForm.get({1, 2}, {-1, -1});
        ASSERT_EQ(freeA1freeB0.size(), 1);
        EXPECT_EQ(freeA1freeB0[0], alice_b0_bob_a0->Id());

        auto freeA1freeB1 = cgForm.get({1, 3}, {-1, -1});
        ASSERT_EQ(freeA1freeB1.size(), 1);
        EXPECT_EQ(freeA1freeB1[0], alice_b0_bob_b0->Id());
    }

    TEST(CollinsGisin, GetWithFixed223) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 2, 3));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto &alice = contextPtr->Parties[0];
        const auto &bob = contextPtr->Parties[1];

        MatrixSystem system{contextPtr};
        auto& momentMatrix = system.CreateMomentMatrix(1);
        
        auto alice_a0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        auto alice_a0_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(0, 1)}, contextPtr.get()));
        auto alice_a0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        auto alice_a0_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 0), bob.measurement_outcome(1, 1)}, contextPtr.get()));
        ASSERT_NE(alice_a0_bob_a0, nullptr);
        ASSERT_NE(alice_a0_bob_a1, nullptr);
        ASSERT_NE(alice_a0_bob_b0, nullptr);
        ASSERT_NE(alice_a0_bob_b1, nullptr);

        auto alice_a1_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        auto alice_a1_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(0, 1)}, contextPtr.get()));
        auto alice_a1_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        auto alice_a1_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(0, 1), bob.measurement_outcome(1, 1)}, contextPtr.get()));
        ASSERT_NE(alice_a1_bob_a0, nullptr);
        ASSERT_NE(alice_a1_bob_a1, nullptr);
        ASSERT_NE(alice_a1_bob_b0, nullptr);
        ASSERT_NE(alice_a1_bob_b1, nullptr);
        
        auto alice_b0_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        auto alice_b0_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(0, 1)}, contextPtr.get()));
        auto alice_b0_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        auto alice_b0_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 0), bob.measurement_outcome(1, 1)}, contextPtr.get()));
        ASSERT_NE(alice_b0_bob_a0, nullptr);
        ASSERT_NE(alice_b0_bob_a1, nullptr);
        ASSERT_NE(alice_b0_bob_b0, nullptr);
        ASSERT_NE(alice_b0_bob_b1, nullptr);

        auto alice_b1_bob_a0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(0, 0)}, contextPtr.get()));
        auto alice_b1_bob_a1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(0, 1)}, contextPtr.get()));
        auto alice_b1_bob_b0 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(1, 0)}, contextPtr.get()));
        auto alice_b1_bob_b1 = momentMatrix.Symbols.where(
                OperatorSequence({alice.measurement_outcome(1, 1), bob.measurement_outcome(1, 1)}, contextPtr.get()));
        ASSERT_NE(alice_b1_bob_a0, nullptr);
        ASSERT_NE(alice_b1_bob_a1, nullptr);
        ASSERT_NE(alice_b1_bob_b0, nullptr);
        ASSERT_NE(alice_b1_bob_b1, nullptr);

        const CollinsGisinForm& cgForm = momentMatrix.CollinsGisin();
        ASSERT_EQ(cgForm.Level, 2);

        auto fixA00freeB0 = cgForm.get({0, 2}, {0, -1});
        ASSERT_EQ(fixA00freeB0.size(), 2);
        EXPECT_EQ(fixA00freeB0[0], alice_a0_bob_a0->Id());
        EXPECT_EQ(fixA00freeB0[1], alice_a0_bob_a1->Id());

        auto fixA00freeB1 = cgForm.get({0, 3}, {0, -1});
        ASSERT_EQ(fixA00freeB1.size(), 2);
        EXPECT_EQ(fixA00freeB1[0], alice_a0_bob_b0->Id());
        EXPECT_EQ(fixA00freeB1[1], alice_a0_bob_b1->Id());

        auto fixA01freeB0 = cgForm.get({0, 2}, {1, -1});
        ASSERT_EQ(fixA01freeB0.size(), 2);
        EXPECT_EQ(fixA01freeB0[0], alice_a1_bob_a0->Id());
        EXPECT_EQ(fixA01freeB0[1], alice_a1_bob_a1->Id());

        auto fixA01freeB1 = cgForm.get({0, 3}, {1, -1});
        ASSERT_EQ(fixA01freeB1.size(), 2);
        EXPECT_EQ(fixA01freeB1[0], alice_a1_bob_b0->Id());
        EXPECT_EQ(fixA01freeB1[1], alice_a1_bob_b1->Id());

        auto fixA10freeB0 = cgForm.get({1, 2}, {0, -1});
        ASSERT_EQ(fixA10freeB0.size(), 2);
        EXPECT_EQ(fixA10freeB0[0], alice_b0_bob_a0->Id());
        EXPECT_EQ(fixA10freeB0[1], alice_b0_bob_a1->Id());

        auto fixA10freeB1 = cgForm.get({1, 3}, {0, -1});
        ASSERT_EQ(fixA10freeB1.size(), 2);
        EXPECT_EQ(fixA10freeB1[0], alice_b0_bob_b0->Id());
        EXPECT_EQ(fixA10freeB1[1], alice_b0_bob_b1->Id());

        auto fixA11freeB0 = cgForm.get({1, 2}, {1, -1});
        ASSERT_EQ(fixA11freeB0.size(), 2);
        EXPECT_EQ(fixA11freeB0[0], alice_b1_bob_a0->Id());
        EXPECT_EQ(fixA11freeB0[1], alice_b1_bob_a1->Id());

        auto fixA11freeB1 = cgForm.get({1, 3}, {1, -1});
        ASSERT_EQ(fixA11freeB1.size(), 2);
        EXPECT_EQ(fixA11freeB1[0], alice_b1_bob_b0->Id());
        EXPECT_EQ(fixA11freeB1[1], alice_b1_bob_b1->Id());

        auto freeA0fixB00 = cgForm.get({0, 2}, {-1, 0});
        ASSERT_EQ(freeA0fixB00.size(), 2);
        EXPECT_EQ(freeA0fixB00[0], alice_a0_bob_a0->Id());
        EXPECT_EQ(freeA0fixB00[1], alice_a1_bob_a0->Id());
        
        auto freeA0fixB01 = cgForm.get({0, 2}, {-1, 1});
        ASSERT_EQ(freeA0fixB01.size(), 2);
        EXPECT_EQ(freeA0fixB01[0], alice_a0_bob_a1->Id());
        EXPECT_EQ(freeA0fixB01[1], alice_a1_bob_a1->Id());
        
        auto freeA0fixB10 = cgForm.get({0, 3}, {-1, 0});
        ASSERT_EQ(freeA0fixB10.size(), 2);
        EXPECT_EQ(freeA0fixB10[0], alice_a0_bob_b0->Id());
        EXPECT_EQ(freeA0fixB10[1], alice_a1_bob_b0->Id());

        auto freeA0fixB11 = cgForm.get({0, 3}, {-1, 1});
        ASSERT_EQ(freeA0fixB11.size(), 2);
        EXPECT_EQ(freeA0fixB11[0], alice_a0_bob_b1->Id());
        EXPECT_EQ(freeA0fixB11[1], alice_a1_bob_b1->Id());
        
        auto freeA1fixB00 = cgForm.get({1, 2}, {-1, 0});
        ASSERT_EQ(freeA1fixB00.size(), 2);
        EXPECT_EQ(freeA1fixB00[0], alice_b0_bob_a0->Id());
        EXPECT_EQ(freeA1fixB00[1], alice_b1_bob_a0->Id());
        
        auto freeA1fixB01 = cgForm.get({1, 2}, {-1, 1});
        ASSERT_EQ(freeA1fixB01.size(), 2);
        EXPECT_EQ(freeA1fixB01[0], alice_b0_bob_a1->Id());
        EXPECT_EQ(freeA1fixB01[1], alice_b1_bob_a1->Id());
        
        auto freeA1fixB10 = cgForm.get({1, 3}, {-1, 0});
        ASSERT_EQ(freeA1fixB10.size(), 2);
        EXPECT_EQ(freeA1fixB10[0], alice_b0_bob_b0->Id());
        EXPECT_EQ(freeA1fixB10[1], alice_b1_bob_b0->Id());

        auto freeA1fixB11 = cgForm.get({1, 3}, {-1, 1});
        ASSERT_EQ(freeA1fixB11.size(), 2);
        EXPECT_EQ(freeA1fixB11[0], alice_b0_bob_b1->Id());
        EXPECT_EQ(freeA1fixB11[1], alice_b1_bob_b1->Id());
    }

    TEST(CollinsGisin, VariedOutcomes_52_22_32) {
        std::vector<Party> partyList;
        partyList.emplace_back(0, "a");
        partyList.emplace_back(1, "b");
        partyList.emplace_back(2, "c");
        partyList[0].add_measurement(Measurement("a", 5));
        partyList[0].add_measurement(Measurement("b", 2));
        partyList[1].add_measurement(Measurement("a", 2));
        partyList[1].add_measurement(Measurement("b", 2));
        partyList[2].add_measurement(Measurement("a", 3));
        partyList[2].add_measurement(Measurement("b", 2));

        auto contextPtr = std::make_shared<Context>(std::move(partyList));
        ASSERT_EQ(contextPtr->Parties.size(), 3);
        const auto &alice = contextPtr->Parties[0];
        const auto &bob = contextPtr->Parties[1];
        const auto &charlie = contextPtr->Parties[2];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(charlie.Measurements.size(), 2);

        MatrixSystem system{contextPtr};
        auto& momentMatrix = system.CreateMomentMatrix(2);
        const auto& cgForm = momentMatrix.CollinsGisin();

        auto aaa = cgForm.get({0, 2, 4});
        EXPECT_EQ(aaa.size(), 4*1*2); // 5, 2, 3

        auto a0axax = cgForm.get({0, 2, 4}, {3, -1, -1});
        ASSERT_EQ(a0axax.size(), 2); // [a0a0a0, a0a0a1]

    }
}