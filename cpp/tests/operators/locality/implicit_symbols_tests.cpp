/**
 * implicit_symbols_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/locality/locality_context.h"
#include "operators/locality/locality_matrix_system.h"
#include "operators/locality/locality_implicit_symbols.h"
#include "operators/matrix/moment_matrix.h"

namespace NPATK::Tests {

    namespace {
        void test2Mmt(std::span<const PMODefinition> spanA,
                      symbol_name_t id, symbol_name_t alice,
                      const std::string& ctx = "") {
            ASSERT_FALSE(spanA.empty()) << ctx;
            ASSERT_EQ(spanA.size(), 2) << ctx;

            EXPECT_EQ(spanA[0].symbol_id, alice) << ctx;
            ASSERT_EQ(spanA[0].expression.size(), 1) << ctx;
            EXPECT_EQ(spanA[0].expression[0].first, alice) << ctx;
            EXPECT_EQ(spanA[0].expression[0].second, 1.0) << ctx;

            EXPECT_EQ(spanA[1].symbol_id, -1) << ctx;
            ASSERT_EQ(spanA[1].expression.size(), 2) << ctx;
            EXPECT_EQ(spanA[1].expression[0].first, id) << ctx;
            EXPECT_EQ(spanA[1].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanA[1].expression[1].first, alice) << ctx;
            EXPECT_EQ(spanA[1].expression[1].second, -1.0) << ctx;
        }

        void test3Mmt(std::span<const PMODefinition> spanA,
                      symbol_name_t id, symbol_name_t a0, symbol_name_t a1,
                      const std::string& ctx = "") {
            ASSERT_FALSE(spanA.empty()) << ctx;
            ASSERT_EQ(spanA.size(), 3) << ctx;

            EXPECT_EQ(spanA[0].symbol_id, a0) << ctx;
            ASSERT_EQ(spanA[0].expression.size(), 1) << ctx;
            EXPECT_EQ(spanA[0].expression[0].first, a0) << ctx;
            EXPECT_EQ(spanA[0].expression[0].second, 1.0) << ctx;

            EXPECT_EQ(spanA[1].symbol_id, a1) << ctx;
            ASSERT_EQ(spanA[1].expression.size(), 1) << ctx;
            EXPECT_EQ(spanA[1].expression[0].first, a1) << ctx;
            EXPECT_EQ(spanA[1].expression[0].second, 1.0) << ctx;

            EXPECT_EQ(spanA[2].symbol_id, -1) << ctx;
            ASSERT_EQ(spanA[2].expression.size(), 3) << ctx;
            EXPECT_EQ(spanA[2].expression[0].first, id) << ctx;
            EXPECT_EQ(spanA[2].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanA[2].expression[1].first, a0) << ctx;
            EXPECT_EQ(spanA[2].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanA[2].expression[2].first, a1) << ctx;
            EXPECT_EQ(spanA[2].expression[2].second, -1.0) << ctx;
        }

        void test22JoinMmt(std::span<const PMODefinition> spanAB,
                           symbol_name_t id,
                           symbol_name_t alice,
                           symbol_name_t bob,
                           symbol_name_t alice_bob,
                           const std::string& ctx = "") {
            ASSERT_FALSE(spanAB.empty()) << ctx;
            ASSERT_EQ(spanAB.size(), 4) << ctx;
            EXPECT_EQ(spanAB[0].symbol_id, alice_bob) << ctx;
            ASSERT_EQ(spanAB[0].expression.size(), 1) << ctx;
            EXPECT_EQ(spanAB[0].expression[0].first, alice_bob) << ctx;
            EXPECT_EQ(spanAB[0].expression[0].second, 1.0) << ctx;

            EXPECT_EQ(spanAB[1].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[1].expression.size(), 2) << ctx;
            EXPECT_EQ(spanAB[1].expression[0].first, alice) << ctx;
            EXPECT_EQ(spanAB[1].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[1].expression[1].first, alice_bob) << ctx;
            EXPECT_EQ(spanAB[1].expression[1].second, -1.0) << ctx;

            EXPECT_EQ(spanAB[2].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[2].expression.size(), 2) << ctx;
            EXPECT_EQ(spanAB[2].expression[0].first, bob) << ctx;
            EXPECT_EQ(spanAB[2].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[2].expression[1].first, alice_bob) << ctx;
            EXPECT_EQ(spanAB[2].expression[1].second, -1.0) << ctx;

            EXPECT_EQ(spanAB[3].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[3].expression.size(), 4) << ctx;
            EXPECT_EQ(spanAB[3].expression[0].first, id) << ctx; // ID
            EXPECT_EQ(spanAB[3].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[3].expression[1].first, alice) << ctx;
            EXPECT_EQ(spanAB[3].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanAB[3].expression[2].first, bob) << ctx;
            EXPECT_EQ(spanAB[3].expression[2].second, -1.0) << ctx;
            EXPECT_EQ(spanAB[3].expression[3].first, alice_bob) << ctx;
            EXPECT_EQ(spanAB[3].expression[3].second, 1.0) << ctx;
        }

        void test32JoinMmt(std::span<const PMODefinition> spanAB,
                           symbol_name_t id,
                           symbol_name_t a0,
                           symbol_name_t a1,
                           symbol_name_t b,
                           symbol_name_t a0b,
                           symbol_name_t a1b,
                           const std::string& ctx = "") {
            ASSERT_FALSE(spanAB.empty()) << ctx;
            ASSERT_EQ(spanAB.size(), 6) << ctx;

            // a0b0 (expl.)
            EXPECT_EQ(spanAB[0].symbol_id, a0b) << ctx;
            ASSERT_EQ(spanAB[0].expression.size(), 1) << ctx;
            EXPECT_EQ(spanAB[0].expression[0].first, a0b) << ctx;
            EXPECT_EQ(spanAB[0].expression[0].second, 1.0) << ctx;

            // a0b1 = a0 - a0b0
            EXPECT_EQ(spanAB[1].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[1].expression.size(), 2) << ctx;
            EXPECT_EQ(spanAB[1].expression[0].first, a0) << ctx;
            EXPECT_EQ(spanAB[1].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[1].expression[1].first, a0b) << ctx;
            EXPECT_EQ(spanAB[1].expression[1].second, -1.0) << ctx;

            // a1b0 (expl.)
            EXPECT_EQ(spanAB[2].symbol_id, a1b) << ctx;
            ASSERT_EQ(spanAB[2].expression.size(), 1) << ctx;
            EXPECT_EQ(spanAB[2].expression[0].first, a1b) << ctx;
            EXPECT_EQ(spanAB[2].expression[0].second, 1.0) << ctx;

            // a1b1 = a1 - a1b0
            EXPECT_EQ(spanAB[3].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[3].expression.size(), 2) << ctx;
            EXPECT_EQ(spanAB[3].expression[0].first, a1) << ctx;
            EXPECT_EQ(spanAB[3].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[3].expression[1].first, a1b) << ctx;
            EXPECT_EQ(spanAB[3].expression[1].second, -1.0) << ctx;

            // a2b0 = b0 - a0b0 - a1b0
            EXPECT_EQ(spanAB[4].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[4].expression.size(), 3) << ctx;
            EXPECT_EQ(spanAB[4].expression[0].first, b) << ctx; // ID
            EXPECT_EQ(spanAB[4].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[4].expression[1].first, a0b) << ctx;
            EXPECT_EQ(spanAB[4].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanAB[4].expression[2].first, a1b) << ctx;
            EXPECT_EQ(spanAB[4].expression[2].second, -1.0) << ctx;

            // a2b1 = 1 - a0 - a1 - b0 + a0b0 + a1b0
            EXPECT_EQ(spanAB[5].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[5].expression.size(), 6) << ctx;
            EXPECT_EQ(spanAB[5].expression[0].first, id) << ctx; // ID
            EXPECT_EQ(spanAB[5].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[5].expression[1].first, a0) << ctx;
            EXPECT_EQ(spanAB[5].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanAB[5].expression[2].first, a1) << ctx;
            EXPECT_EQ(spanAB[5].expression[2].second, -1.0) << ctx;
            EXPECT_EQ(spanAB[5].expression[3].first, b) << ctx;
            EXPECT_EQ(spanAB[5].expression[3].second, -1.0) << ctx;
            EXPECT_EQ(spanAB[5].expression[4].first, a0b) << ctx;
            EXPECT_EQ(spanAB[5].expression[4].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[5].expression[5].first, a1b) << ctx;
            EXPECT_EQ(spanAB[5].expression[5].second, 1.0) << ctx;
        }

        void test222JoinMmt(std::span<const PMODefinition> spanABC,
                           symbol_name_t id,
                           symbol_name_t alice,
                           symbol_name_t bob,
                           symbol_name_t charlie,
                           symbol_name_t alice_bob,
                           symbol_name_t alice_charlie,
                           symbol_name_t bob_charlie,
                           symbol_name_t alice_bob_charlie,
                           const std::string& ctx = "") {
            // 8 outcomes, most implicit...!

            ASSERT_FALSE(spanABC.empty()) << ctx;
            ASSERT_EQ(spanABC.size(), 8) << ctx;

            // a0b0c0
            EXPECT_EQ(spanABC[0].symbol_id, alice_bob_charlie) << ctx;
            ASSERT_EQ(spanABC[0].expression.size(), 1) << ctx;
            EXPECT_EQ(spanABC[0].expression[0].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[0].expression[0].second, 1.0) << ctx;

            // a0b0c1 = a0b0 - a0b0c0
            EXPECT_EQ(spanABC[1].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[1].expression.size(), 2) << ctx;
            EXPECT_EQ(spanABC[1].expression[0].first, alice_bob) << ctx;
            EXPECT_EQ(spanABC[1].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[1].expression[1].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[1].expression[1].second, -1.0) << ctx;

            // a0b1c0 = a0c0 - a0b0c0
            EXPECT_EQ(spanABC[2].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[2].expression.size(), 2) << ctx;
            EXPECT_EQ(spanABC[2].expression[0].first, alice_charlie) << ctx;
            EXPECT_EQ(spanABC[2].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[2].expression[1].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[2].expression[1].second, -1.0) << ctx;

            // a0b1c1 = a0 - a0b0 - a0c0 + a0b0c0
            EXPECT_EQ(spanABC[3].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[3].expression.size(), 4) << ctx;
            EXPECT_EQ(spanABC[3].expression[0].first, alice) << ctx;
            EXPECT_EQ(spanABC[3].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[3].expression[1].first, alice_bob) << ctx;
            EXPECT_EQ(spanABC[3].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[3].expression[2].first, alice_charlie) << ctx;
            EXPECT_EQ(spanABC[3].expression[2].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[3].expression[3].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[3].expression[3].second, +1.0) << ctx;

            // a1b0c0 = b0c0 - a0b0c0
            EXPECT_EQ(spanABC[4].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[4].expression.size(), 2) << ctx;
            EXPECT_EQ(spanABC[4].expression[0].first, bob_charlie) << ctx;
            EXPECT_EQ(spanABC[4].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[4].expression[1].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[4].expression[1].second, -1.0) << ctx;

            // a1b0c1 = b0 - a0b0 - b0c0 + a0b0c0
            EXPECT_EQ(spanABC[5].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[5].expression.size(), 4) << ctx;
            EXPECT_EQ(spanABC[5].expression[0].first, bob) << ctx;
            EXPECT_EQ(spanABC[5].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[5].expression[1].first, alice_bob) << ctx;
            EXPECT_EQ(spanABC[5].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[5].expression[2].first, bob_charlie) << ctx;
            EXPECT_EQ(spanABC[5].expression[2].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[5].expression[3].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[5].expression[3].second, +1.0) << ctx;
            
            // a1b1c0 = c0 - a0c0 - b0c0 + a0b0c0
            EXPECT_EQ(spanABC[6].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[6].expression.size(), 4) << ctx;
            EXPECT_EQ(spanABC[6].expression[0].first, charlie) << ctx;
            EXPECT_EQ(spanABC[6].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[6].expression[1].first, alice_charlie) << ctx;
            EXPECT_EQ(spanABC[6].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[6].expression[2].first, bob_charlie) << ctx;
            EXPECT_EQ(spanABC[6].expression[2].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[6].expression[3].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[6].expression[3].second, +1.0) << ctx;

            // a1b1c1 = 1 - a0 - b0 - c0 + a0b0 + a0c0 + b0c0 - a0b0c0
            EXPECT_EQ(spanABC[7].symbol_id, -1) << ctx;
            ASSERT_EQ(spanABC[7].expression.size(), 8) << ctx;
            EXPECT_EQ(spanABC[7].expression[0].first, id) << ctx;
            EXPECT_EQ(spanABC[7].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[1].first, alice) << ctx;
            EXPECT_EQ(spanABC[7].expression[1].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[2].first, bob) << ctx;
            EXPECT_EQ(spanABC[7].expression[2].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[3].first, charlie) << ctx;
            EXPECT_EQ(spanABC[7].expression[3].second, -1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[4].first, alice_bob) << ctx;
            EXPECT_EQ(spanABC[7].expression[4].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[5].first, alice_charlie) << ctx;
            EXPECT_EQ(spanABC[7].expression[5].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[6].first, bob_charlie) << ctx;
            EXPECT_EQ(spanABC[7].expression[6].second, 1.0) << ctx;
            EXPECT_EQ(spanABC[7].expression[7].first, alice_bob_charlie) << ctx;
            EXPECT_EQ(spanABC[7].expression[7].second, -1.0) << ctx;
        }
    }

    TEST(ImplicitSymbols, Empty) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>()};
        auto [eid, emptyMM] = system.create_moment_matrix(1);

        const auto& implSym = system.ImplicitSymbolTable();
        //ImplicitSymbols implSym{system};

        EXPECT_EQ(implSym.MaxSequenceLength, 0);
        ASSERT_FALSE(implSym.Data().empty());
        ASSERT_EQ(implSym.Data().size(), 1);

        const auto& one = implSym.Data().front();
        EXPECT_EQ(one.symbol_id, 1);
        SymbolCombo oneCombo{{1,1.0}};
        EXPECT_EQ(one.expression, oneCombo);

        const auto& getOne = implSym.get(std::vector<PMOIndex>{});
        EXPECT_EQ(getOne.symbol_id, 1);
        EXPECT_EQ(&getOne, &one);
    }

    TEST(ImplicitSymbols, OnePartyOneMmt) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 1, 3))};
        const auto& context = system.localityContext;

        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);

        auto [id, momentMatrix] = system.create_moment_matrix(1);

        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
        auto where_a0 = momentMatrix.Symbols.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& alice_a1 = OperatorSequence({alice.measurement_outcome(0,1)}, context);
        auto where_a1 = momentMatrix.Symbols.where(alice_a1);
        ASSERT_NE(where_a1, nullptr);
        ASSERT_NE(where_a0, where_a1);

        const auto& implSym = system.ImplicitSymbolTable();
        EXPECT_EQ(implSym.MaxSequenceLength, 1);

        std::vector<size_t> indices{0};
        auto pmoSpan = implSym.get(indices);
        ASSERT_FALSE(pmoSpan.empty());
        ASSERT_EQ(pmoSpan.size(), 3);

        EXPECT_EQ(pmoSpan[0].symbol_id, where_a0->Id());
        ASSERT_EQ(pmoSpan[0].expression.size(), 1);
        EXPECT_EQ(pmoSpan[0].expression[0].first, where_a0->Id());
        EXPECT_EQ(pmoSpan[0].expression[0].second, 1.0);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}}), &pmoSpan[0]);

        EXPECT_EQ(pmoSpan[1].symbol_id, where_a1->Id());
        ASSERT_EQ(pmoSpan[1].expression.size(), 1);
        EXPECT_EQ(pmoSpan[1].expression[0].first, where_a1->Id());
        EXPECT_EQ(pmoSpan[1].expression[0].second, 1.0);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}}), &pmoSpan[1]);

        EXPECT_EQ(pmoSpan[2].symbol_id, -1);
        ASSERT_EQ(pmoSpan[2].expression.size(), 3);
        EXPECT_EQ(pmoSpan[2].expression[0].first, 1);
        EXPECT_EQ(pmoSpan[2].expression[0].second, 1.0);
        EXPECT_EQ(pmoSpan[2].expression[1].first, where_a0->Id());
        EXPECT_EQ(pmoSpan[2].expression[1].second, -1.0);
        EXPECT_EQ(pmoSpan[2].expression[2].first, where_a1->Id());
        EXPECT_EQ(pmoSpan[2].expression[2].second, -1.0);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,2}}), &pmoSpan[2]);
    }

    TEST(ImplicitSymbols, OnePartyTwoMmt) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(1, 2, 2))};
        const auto& context = system.localityContext;
        const auto& alice = context.Parties[0];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);


        auto [id, momentMatrix] = system.create_moment_matrix(1);

        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
        auto where_a0 = momentMatrix.Symbols.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& alice_b0 = OperatorSequence({alice.measurement_outcome(1,0)}, context);
        auto where_b0 = momentMatrix.Symbols.where(alice_b0);
        ASSERT_NE(where_b0, nullptr);
        ASSERT_NE(where_a0, where_b0);

        const auto& implSym = system.ImplicitSymbolTable();
        EXPECT_EQ(implSym.MaxSequenceLength, 1);

        auto spanA = implSym.get({0});
        test2Mmt(spanA, 1, where_a0->Id(), "a0");
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}}), &spanA[0]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}}), &spanA[1]);

        auto spanB = implSym.get({1});
        test2Mmt(spanB, 1, where_b0->Id(), "b0");
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,1,0}}), &spanB[0]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,1,1}}), &spanB[1]);
    }


    TEST(ImplicitSymbols, TwoPartyOneMmtEach) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 1, 2))};
        const auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 1);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);


        auto [id, momentMatrix] = system.create_moment_matrix(1);

        const auto& alice_a0 = OperatorSequence({alice.measurement_outcome(0,0)}, context);
        auto where_a0 = momentMatrix.Symbols.where(alice_a0);
        ASSERT_NE(where_a0, nullptr);
        const auto& bob_a0 = OperatorSequence({bob.measurement_outcome(0,0)}, context);
        auto where_b0 = momentMatrix.Symbols.where(bob_a0);
        ASSERT_NE(where_b0, nullptr);
        ASSERT_NE(where_a0, where_b0);
        const auto& alice_a0_bob_a0 = OperatorSequence({alice.measurement_outcome(0,0), bob.measurement_outcome(0,0)},
                                                       context);
        auto where_alice_bob = momentMatrix.Symbols.where(alice_a0_bob_a0);
        ASSERT_NE(where_alice_bob, nullptr);
        ASSERT_NE(where_alice_bob, where_a0);
        ASSERT_NE(where_alice_bob, where_b0);

        const auto& implSym = system.ImplicitSymbolTable();
        EXPECT_EQ(implSym.MaxSequenceLength, 2);

        // Alice a
        auto spanA = implSym.get({0});
        test2Mmt(spanA, 1, where_a0->Id(), "a0");
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}}), &spanA[0]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}}), &spanA[1]);

        // Bob b
        auto spanB = implSym.get({1});
        test2Mmt(spanB, 1, where_b0->Id(), "b0");
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{1,0,0}}), &spanB[0]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{1,0,1}}), &spanB[1]);

        // Alice a, Bob b
        auto spanAB = implSym.get({0, 1});
        test22JoinMmt(spanAB, 1, where_a0->Id(), where_b0->Id(), where_alice_bob->Id(), "AB");
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}, PMOIndex{1,0,0}}), &spanAB[0]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,0}, PMOIndex{1,0,1}}), &spanAB[1]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}, PMOIndex{1,0,0}}), &spanAB[2]);
        EXPECT_EQ(&implSym.get(std::vector{PMOIndex{0,0,1}, PMOIndex{1,0,1}}), &spanAB[3]);
    }

    TEST(ImplicitSymbols, CHSH) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(2, 2, 2))};
        const auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 2);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);

        auto [id, momentMatrix] = system.create_moment_matrix(1);

        auto A0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                                      context))->Id();
        auto A1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1, 0)},
                                                                      context))->Id();
        auto B0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                                      context))->Id();
        auto B1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1, 0)},
                                                                      context))->Id();
        auto A0B0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                         bob.measurement_outcome(0, 0)},
                                                                        context))->Id();
        auto A0B1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                         bob.measurement_outcome(1, 0)},
                                                                        context))->Id();
        auto A1B0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                         bob.measurement_outcome(0, 0)},
                                                                        context))->Id();
        auto A1B1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1, 0),
                                                                         bob.measurement_outcome(1, 0)},
                                                                        context))->Id();
        const auto& implSym = system.ImplicitSymbolTable();


        auto spanA0 = implSym.get({0});
        test2Mmt(spanA0, 1, A0, "A0");

        auto spanA1 = implSym.get({1});
        test2Mmt(spanA1, 1, A1, "A1");

        auto spanB0 = implSym.get({2});
        test2Mmt(spanB0, 1, B0, "B0");

        auto spanB1 = implSym.get({3});
        test2Mmt(spanB1, 1, B1, "B1");

        // Alice a, Bob b
        auto spanA0B0 = implSym.get({0, 2});
        test22JoinMmt(spanA0B0, 1, A0, B0, A0B0, "A0B0");

        auto spanA0B1 = implSym.get({0, 3});
        test22JoinMmt(spanA0B1, 1, A0, B1, A0B1, "A0B1");

        auto spanA1B0 = implSym.get({1, 2});
        test22JoinMmt(spanA1B0, 1, A1, B0, A1B0, "A1B0");

        auto spanA1B1 = implSym.get({1, 3});
        test22JoinMmt(spanA1B1, 1, A1, B1, A1B1, "A1B1");
    }


    TEST(ImplicitSymbols, Tripartite322) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 2, 2))};
        const auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 3);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        const auto& charlie = context.Parties[2];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(charlie.Measurements.size(), 2);
        ASSERT_EQ(charlie.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(charlie.Measurements[1].num_outcomes, 2);

        auto [id, momentMatrix] = system.create_moment_matrix(2);

        auto A0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0)},
                                                                       context))->Id();
        auto A1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0)},
                                                                       context))->Id();
        auto B0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0,0)},
                                                                       context))->Id();
        auto B1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1,0)},
                                                                       context))->Id();
        auto C0 = momentMatrix.Symbols.where(OperatorSequence({charlie.measurement_outcome(0,0)},
                                                                       context))->Id();
        auto C1 = momentMatrix.Symbols.where(OperatorSequence({charlie.measurement_outcome(1,0)},
                                                                       context))->Id();

        auto A0B0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         bob.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A0B1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         bob.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto A0C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A0C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto A1B0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         bob.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A1B1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         bob.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto A1C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A1C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();

        auto B0C0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto B0C1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto B1C0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto B1C1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();

        auto A0B0C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                           bob.measurement_outcome(0,0),
                                                                           charlie.measurement_outcome(0,0)},
                                                                        context))->Id();
        auto A0B0C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                           bob.measurement_outcome(0,0),
                                                                           charlie.measurement_outcome(1,0)},
                                                                        context))->Id();
        auto A0B1C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                           bob.measurement_outcome(1,0),
                                                                           charlie.measurement_outcome(0,0)},
                                                                        context))->Id();
        auto A0B1C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                           bob.measurement_outcome(1,0),
                                                                           charlie.measurement_outcome(1,0)},
                                                                        context))->Id();
        auto A1B0C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                           bob.measurement_outcome(0,0),
                                                                           charlie.measurement_outcome(0,0)},
                                                                        context))->Id();
        auto A1B0C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                           bob.measurement_outcome(0,0),
                                                                           charlie.measurement_outcome(1,0)},
                                                                        context))->Id();
        auto A1B1C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                           bob.measurement_outcome(1,0),
                                                                           charlie.measurement_outcome(0,0)},
                                                                        context))->Id();
        auto A1B1C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                           bob.measurement_outcome(1,0),
                                                                           charlie.measurement_outcome(1,0)},
                                                                        context))->Id();

        const auto& implSym = system.ImplicitSymbolTable();

        // MONOPARTITE TESTS:
        auto spanA0 = implSym.get({0});
        test2Mmt(spanA0, 1, A0, "A0");

        auto spanA1 = implSym.get({1});
        test2Mmt(spanA1, 1, A1, "A1");

        auto spanB0 = implSym.get({2});
        test2Mmt(spanB0, 1, B0, "B0");

        auto spanB1 = implSym.get({3});
        test2Mmt(spanB1, 1, B1, "B1");

        auto spanC0 = implSym.get({4});
        test2Mmt(spanC0, 1, C0, "C0");

        auto spanC1 = implSym.get({5});
        test2Mmt(spanC1, 1, C1, "C1");

        // BIPARTITE TESTS:
        const auto spanA0B0 = implSym.get({0, 2});
        test22JoinMmt(spanA0B0, 1, A0, B0, A0B0, "A0B0");

        const auto spanA0B1 = implSym.get({0, 3});
        test22JoinMmt(spanA0B1, 1, A0, B1, A0B1, "A0B1");

        const auto spanA1B0 = implSym.get({1, 2});
        test22JoinMmt(spanA1B0, 1, A1, B0, A1B0, "A1B0");

        const auto spanA1B1 = implSym.get({1, 3});
        test22JoinMmt(spanA1B1, 1, A1, B1, A1B1, "A1B1");

        const auto spanA0C0 = implSym.get({0, 4});
        test22JoinMmt(spanA0C0, 1, A0, C0, A0C0, "A0C0");

        const auto spanA0C1 = implSym.get({0, 5});
        test22JoinMmt(spanA0C1, 1, A0, C1, A0C1, "A0C1");

        const auto spanA1C0 = implSym.get({1, 4});
        test22JoinMmt(spanA1C0, 1, A1, C0, A1C0, "A1C0");

        const auto spanA1C1 = implSym.get({1, 5});
        test22JoinMmt(spanA1C1, 1, A1, C1, A1C1, "A1C1");
        
        const auto spanB0C0 = implSym.get({2, 4});
        test22JoinMmt(spanB0C0, 1, B0, C0, B0C0, "B0C0");

        const auto spanB0C1 = implSym.get({2, 5});
        test22JoinMmt(spanB0C1, 1, B0, C1, B0C1, "B0C1");

        const auto spanB1C0 = implSym.get({3, 4});
        test22JoinMmt(spanB1C0, 1, B1, C0, B1C0, "B0C0");

        const auto spanB1C1 = implSym.get({3, 5});
        test22JoinMmt(spanB1C1, 1, B1, C1, B1C1, "B1C1");

        // TRIPARTITE TESTS
        const auto spanA0B0C0 = implSym.get({0, 2, 4});
        test222JoinMmt(spanA0B0C0, 1, A0, B0, C0, A0B0, A0C0, B0C0, A0B0C0, "A0B0C0");

        const auto spanA0B0C1 = implSym.get({0, 2, 5});
        test222JoinMmt(spanA0B0C1, 1, A0, B0, C1, A0B0, A0C1, B0C1, A0B0C1, "A0B0C1");

        const auto spanA0B1C0 = implSym.get({0, 3, 4});
        test222JoinMmt(spanA0B1C0, 1, A0, B1, C0, A0B1, A0C0, B1C0, A0B1C0, "A0B1C0");

        const auto spanA0B1C1 = implSym.get({0, 3, 5});
        test222JoinMmt(spanA0B1C1, 1, A0, B1, C1, A0B1, A0C1, B1C1, A0B1C1, "A0B1C1");
        
        const auto spanA1B0C0 = implSym.get({1, 2, 4});
        test222JoinMmt(spanA1B0C0, 1, A1, B0, C0, A1B0, A1C0, B0C0, A1B0C0, "A1B0C0");

        const auto spanA1B0C1 = implSym.get({1, 2, 5});
        test222JoinMmt(spanA1B0C1, 1, A1, B0, C1, A1B0, A1C1, B0C1, A1B0C1, "A1B0C1");

        const auto spanA1B1C0 = implSym.get({1, 3, 4});
        test222JoinMmt(spanA1B1C0, 1, A1, B1, C0, A1B1, A1C0, B1C0, A1B1C0, "A1B1C0");

        const auto spanA1B1C1 = implSym.get({1, 3, 5});
        test222JoinMmt(spanA1B1C1, 1, A1, B1, C1, A1B1, A1C1, B1C1, A1B1C1, "A1B1C1");
    }

    TEST(ImplicitSymbols, Tripartite322_LowerMoment) {
        LocalityMatrixSystem system{std::make_unique<LocalityContext>(Party::MakeList(3, 2, 2))};
        const auto& context = system.localityContext;

        ASSERT_EQ(context.Parties.size(), 3);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        const auto& charlie = context.Parties[2];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(alice.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(bob.Measurements[1].num_outcomes, 2);
        ASSERT_EQ(charlie.Measurements.size(), 2);
        ASSERT_EQ(charlie.Measurements[0].num_outcomes, 2);
        ASSERT_EQ(charlie.Measurements[1].num_outcomes, 2);

        auto [id, momentMatrix] = system.create_moment_matrix(1);

        auto A0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0)},
                                                                       context))->Id();
        auto A1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0)},
                                                                       context))->Id();
        auto B0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0,0)},
                                                                       context))->Id();
        auto B1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1,0)},
                                                                       context))->Id();
        auto C0 = momentMatrix.Symbols.where(OperatorSequence({charlie.measurement_outcome(0,0)},
                                                                       context))->Id();
        auto C1 = momentMatrix.Symbols.where(OperatorSequence({charlie.measurement_outcome(1,0)},
                                                                       context))->Id();

        auto A0B0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         bob.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A0B1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         bob.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto A0C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A0C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto A1B0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         bob.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A1B1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         bob.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto A1C0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto A1C1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();

        auto B0C0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto B0C1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();
        auto B1C0 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(0,0)},
                                                                         context))->Id();
        auto B1C1 = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(1,0),
                                                                         charlie.measurement_outcome(1,0)},
                                                                         context))->Id();

        const auto& implSym = system.ImplicitSymbolTable();

        // MONOPARTITE TESTS:
        auto spanA0 = implSym.get({0});
        test2Mmt(spanA0, 1, A0, "A0");

        auto spanA1 = implSym.get({1});
        test2Mmt(spanA1, 1, A1, "A1");

        auto spanB0 = implSym.get({2});
        test2Mmt(spanB0, 1, B0, "B0");

        auto spanB1 = implSym.get({3});
        test2Mmt(spanB1, 1, B1, "B1");

        auto spanC0 = implSym.get({4});
        test2Mmt(spanC0, 1, C0, "C0");

        auto spanC1 = implSym.get({5});
        test2Mmt(spanC1, 1, C1, "C1");

        // BIPARTITE TESTS:
        const auto spanA0B0 = implSym.get({0, 2});
        test22JoinMmt(spanA0B0, 1, A0, B0, A0B0, "A0B0");

        const auto spanA0B1 = implSym.get({0, 3});
        test22JoinMmt(spanA0B1, 1, A0, B1, A0B1, "A0B1");

        const auto spanA1B0 = implSym.get({1, 2});
        test22JoinMmt(spanA1B0, 1, A1, B0, A1B0, "A1B0");

        const auto spanA1B1 = implSym.get({1, 3});
        test22JoinMmt(spanA1B1, 1, A1, B1, A1B1, "A1B1");

        const auto spanA0C0 = implSym.get({0, 4});
        test22JoinMmt(spanA0C0, 1, A0, C0, A0C0, "A0C0");

        const auto spanA0C1 = implSym.get({0, 5});
        test22JoinMmt(spanA0C1, 1, A0, C1, A0C1, "A0C1");

        const auto spanA1C0 = implSym.get({1, 4});
        test22JoinMmt(spanA1C0, 1, A1, C0, A1C0, "A1C0");

        const auto spanA1C1 = implSym.get({1, 5});
        test22JoinMmt(spanA1C1, 1, A1, C1, A1C1, "A1C1");

        const auto spanB0C0 = implSym.get({2, 4});
        test22JoinMmt(spanB0C0, 1, B0, C0, B0C0, "B0C0");

        const auto spanB0C1 = implSym.get({2, 5});
        test22JoinMmt(spanB0C1, 1, B0, C1, B0C1, "B0C1");

        const auto spanB1C0 = implSym.get({3, 4});
        test22JoinMmt(spanB1C0, 1, B1, C0, B1C0, "B0C0");

        const auto spanB1C1 = implSym.get({3, 5});
        test22JoinMmt(spanB1C1, 1, B1, C1, B1C1, "B1C1");
    }


    TEST(ImplicitSymbols, A13_B12) {

        std::vector<Party> buildParties;
        buildParties.emplace_back(0, "A", std::vector{Measurement("a", 3)});
        buildParties.emplace_back(1, "B", std::vector{Measurement("b", 2)});

        LocalityMatrixSystem system{std::make_unique<LocalityContext>(std::move(buildParties))};
        const auto& context = system.localityContext;
        ASSERT_EQ(context.Parties.size(), 2);
        const auto &alice = context.Parties[0];
        const auto &bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 1);
        ASSERT_EQ(alice.Measurements[0].num_outcomes, 3);
        ASSERT_EQ(bob.Measurements.size(), 1);
        ASSERT_EQ(bob.Measurements[0].num_outcomes, 2);

        auto [id, momentMatrix] = system.create_moment_matrix(1);
        
        auto A0 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 0)},
                                                                      context))->Id();
        auto A1 = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 1)},
                                                                      context))->Id();
        auto B = momentMatrix.Symbols.where(OperatorSequence({bob.measurement_outcome(0, 0)},
                                                                      context))->Id();

        auto A0B = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 0),
                                                                         bob.measurement_outcome(0, 0)},
                                                                        context))->Id();
        auto A1B = momentMatrix.Symbols.where(OperatorSequence({alice.measurement_outcome(0, 1),
                                                                         bob.measurement_outcome(0, 0)},
                                                                        context))->Id();

        const auto& implSym = system.ImplicitSymbolTable();

        // Alice
        auto spanA = implSym.get({0});
        test3Mmt(spanA, 1, A0, A1, "A");

        // Bob
        auto spanB = implSym.get({1});
        test2Mmt(spanB, 1, B, "B");

        // Alice a, Bob b
        auto spanAB = implSym.get({0, 1});
        test32JoinMmt(spanAB, 1, A0, A1, B, A0B, A1B, "AB");
    }

}