/**
 * implicit_symbol_test_helpers.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "implicit_symbol_test_helpers.h"

#include "gtest/gtest.h"




namespace Moment::Tests {
    void test2Mmt(std::span<const PMODefinition> spanA,
                  symbol_name_t id, symbol_name_t alice,
                  const std::string& ctx) {
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
                  const std::string& ctx) {
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
                       const std::string& ctx) {
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

        if (alice == bob) {
            EXPECT_EQ(spanAB[3].symbol_id, -1) << ctx;
            ASSERT_EQ(spanAB[3].expression.size(), 3) << ctx;
            EXPECT_EQ(spanAB[3].expression[0].first, id) << ctx; // ID
            EXPECT_EQ(spanAB[3].expression[0].second, 1.0) << ctx;
            EXPECT_EQ(spanAB[3].expression[1].first, alice) << ctx;
            EXPECT_EQ(spanAB[3].expression[1].second, -2.0) << ctx;
            EXPECT_EQ(spanAB[3].expression[2].first, alice_bob) << ctx;
            EXPECT_EQ(spanAB[3].expression[2].second, 1.0) << ctx;
        } else {
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
    }

    void test32JoinMmt(std::span<const PMODefinition> spanAB,
                       symbol_name_t id,
                       symbol_name_t a0,
                       symbol_name_t a1,
                       symbol_name_t b,
                       symbol_name_t a0b,
                       symbol_name_t a1b,
                       const std::string& ctx) {
        ASSERT_FALSE(spanAB.empty()) << ctx;
        ASSERT_EQ(spanAB.size(), 6) << ctx;

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
                        const std::string& ctx) {
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