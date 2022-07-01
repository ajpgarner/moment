/**
 * implicit_symbols_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/implicit_symbols.h"

namespace NPATK::Tests {
    TEST(ImplicitSymbols, Empty) {
        auto contextPtr = std::make_shared<Context>();
        MomentMatrix emptyMM{contextPtr, 1};

        ImplicitSymbols implSym{emptyMM};

        EXPECT_EQ(implSym.MaxSequenceLength, 0);
        ASSERT_FALSE(implSym.Table().Data().empty());
        ASSERT_EQ(implSym.Table().Data().size(), 1);

        const auto& one = implSym.Table().Data().front();
        EXPECT_EQ(one.symbol_id, 1);
        ImplicitSymbols::SymbolCombo oneCombo{{1,1.0}};
        EXPECT_EQ(one.expression, oneCombo);
    }

    TEST(ImplicitSymbols, OnePartyOneMmt) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(1, 1, 3));
        const auto& alice = contextPtr->Parties[0];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);

        MomentMatrix momentMatrix{contextPtr, 1};
        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, contextPtr.get());
        auto where_a0 = momentMatrix.UniqueSequences.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& alice_a1 = OperatorSequence({alice.measurement_outcome(0,1)}, contextPtr.get());
        auto where_a1 = momentMatrix.UniqueSequences.where(alice_a1);
        ASSERT_NE(where_a1, nullptr);
        ASSERT_NE(where_a0, where_a1);

        ImplicitSymbols implSym{momentMatrix};
        EXPECT_EQ(implSym.MaxSequenceLength, 1);

        const auto& levelOne = implSym.Table();
        std::vector<size_t> indices{0};
        auto pmoSpan = levelOne.get(indices);
        ASSERT_FALSE(pmoSpan.empty());
        ASSERT_EQ(pmoSpan.size(), 3);

        EXPECT_EQ(pmoSpan[0].symbol_id, where_a0->Id());
        ASSERT_EQ(pmoSpan[0].expression.size(), 1);
        EXPECT_EQ(pmoSpan[0].expression[0].first, where_a0->Id());
        EXPECT_EQ(pmoSpan[0].expression[0].second, 1.0);

        EXPECT_EQ(pmoSpan[1].symbol_id, where_a1->Id());
        ASSERT_EQ(pmoSpan[1].expression.size(), 1);
        EXPECT_EQ(pmoSpan[1].expression[0].first, where_a1->Id());
        EXPECT_EQ(pmoSpan[1].expression[0].second, 1.0);

        EXPECT_EQ(pmoSpan[2].symbol_id, -1);
        ASSERT_EQ(pmoSpan[2].expression.size(), 3);
        EXPECT_EQ(pmoSpan[2].expression[0].first, 1);
        EXPECT_EQ(pmoSpan[2].expression[0].second, 1.0);
        EXPECT_EQ(pmoSpan[2].expression[1].first, where_a0->Id());
        EXPECT_EQ(pmoSpan[2].expression[1].second, -1.0);
        EXPECT_EQ(pmoSpan[2].expression[2].first, where_a1->Id());
        EXPECT_EQ(pmoSpan[2].expression[2].second, -1.0);
    }

    TEST(ImplicitSymbols, OnePartyTwoMmt) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(1, 2, 2));
        const auto& alice = contextPtr->Parties[0];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);

        MomentMatrix momentMatrix{contextPtr, 1};
        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, contextPtr.get());
        auto where_a0 = momentMatrix.UniqueSequences.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& alice_b0 = OperatorSequence({alice.measurement_outcome(1,0)}, contextPtr.get());
        auto where_b0 = momentMatrix.UniqueSequences.where(alice_b0);
        ASSERT_NE(where_b0, nullptr);
        ASSERT_NE(where_a0, where_b0);

        ImplicitSymbols implSym{momentMatrix};
        EXPECT_EQ(implSym.MaxSequenceLength, 1);

        const auto& levelOne = implSym.Table();
        std::vector<size_t> indicesA{0};
        auto spanA = levelOne.get(indicesA);
        ASSERT_FALSE(spanA.empty());
        ASSERT_EQ(spanA.size(), 2);

        EXPECT_EQ(spanA[0].symbol_id, where_a0->Id());
        ASSERT_EQ(spanA[0].expression.size(), 1);
        EXPECT_EQ(spanA[0].expression[0].first, where_a0->Id());
        EXPECT_EQ(spanA[0].expression[0].second, 1.0);

        EXPECT_EQ(spanA[1].symbol_id, -1);
        ASSERT_EQ(spanA[1].expression.size(), 2);
        EXPECT_EQ(spanA[1].expression[0].first, 1);
        EXPECT_EQ(spanA[1].expression[0].second, 1.0);
        EXPECT_EQ(spanA[1].expression[1].first, where_a0->Id());
        EXPECT_EQ(spanA[1].expression[1].second, -1.0);

        std::vector<size_t> indicesB{1};
        auto spanB = levelOne.get(indicesB);
        ASSERT_FALSE(spanB.empty());
        ASSERT_EQ(spanB.size(), 2);

        EXPECT_EQ(spanB[0].symbol_id, where_b0->Id());
        ASSERT_EQ(spanB[0].expression.size(), 1);
        EXPECT_EQ(spanB[0].expression[0].first, where_b0->Id());
        EXPECT_EQ(spanB[0].expression[0].second, 1.0);

        EXPECT_EQ(spanB[1].symbol_id, -1);
        ASSERT_EQ(spanB[1].expression.size(), 2);
        EXPECT_EQ(spanB[1].expression[0].first, 1);
        EXPECT_EQ(spanB[1].expression[0].second, 1.0);
        EXPECT_EQ(spanB[1].expression[1].first, where_b0->Id());
        EXPECT_EQ(spanB[1].expression[1].second, -1.0);
    }


    TEST(ImplicitSymbols, TwoPartyOneMmtEach) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 1, 2));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto& alice = contextPtr->Parties[0];
        const auto& bob = contextPtr->Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 1);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);


        MomentMatrix momentMatrix{contextPtr, 1};
        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, contextPtr.get());
        auto where_a0 = momentMatrix.UniqueSequences.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& bob_a0 = OperatorSequence({bob.measurement_outcome(0,0)}, contextPtr.get());
        auto where_b0 = momentMatrix.UniqueSequences.where(bob_a0);
        ASSERT_NE(where_b0, nullptr);
        ASSERT_NE(where_a0, where_b0);

        ImplicitSymbols implSym{momentMatrix};
        EXPECT_EQ(implSym.MaxSequenceLength, 2);

        // Level One
        const auto& levelOne = implSym.Table();

        // Alice a0
        std::vector<size_t> indicesA{0};
        auto spanA = levelOne.get(indicesA);
        ASSERT_FALSE(spanA.empty());
        ASSERT_EQ(spanA.size(), 2);

        EXPECT_EQ(spanA[0].symbol_id, where_a0->Id());
        ASSERT_EQ(spanA[0].expression.size(), 1);
        EXPECT_EQ(spanA[0].expression[0].first, where_a0->Id());
        EXPECT_EQ(spanA[0].expression[0].second, 1.0);

        EXPECT_EQ(spanA[1].symbol_id, -1);
        ASSERT_EQ(spanA[1].expression.size(), 2);
        EXPECT_EQ(spanA[1].expression[0].first, 1);
        EXPECT_EQ(spanA[1].expression[0].second, 1.0);
        EXPECT_EQ(spanA[1].expression[1].first, where_a0->Id());
        EXPECT_EQ(spanA[1].expression[1].second, -1.0);

        // Bob b0
        std::vector<size_t> indicesB{1};
        auto spanB = levelOne.get(indicesB);
        ASSERT_FALSE(spanB.empty());
        ASSERT_EQ(spanB.size(), 2);

        EXPECT_EQ(spanB[0].symbol_id, where_b0->Id());
        ASSERT_EQ(spanB[0].expression.size(), 1);
        EXPECT_EQ(spanB[0].expression[0].first, where_b0->Id());
        EXPECT_EQ(spanB[0].expression[0].second, 1.0);

        EXPECT_EQ(spanB[1].symbol_id, -1);
        ASSERT_EQ(spanB[1].expression.size(), 2);
        EXPECT_EQ(spanB[1].expression[0].first, 1);
        EXPECT_EQ(spanB[1].expression[0].second, 1.0);
        EXPECT_EQ(spanB[1].expression[1].first, where_b0->Id());
        EXPECT_EQ(spanB[1].expression[1].second, -1.0);

        // TODO: Alice Bob a0.b0
    }
}