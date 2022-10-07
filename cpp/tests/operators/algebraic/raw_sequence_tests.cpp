/**
 * algebraic_system_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/algebraic/algebraic_matrix_system.h"
#include "operators/algebraic/algebraic_context.h"

#include <set>

namespace NPATK::Tests {

    TEST(RawSequence, RawSequenceBook) {
        AlgebraicContext theContext{2}; // 2 symbols...
        RawSequenceBook rsb{theContext};

        ASSERT_EQ(rsb.size(), 2);
        EXPECT_EQ(rsb.longest_sequence(), 0);


        rsb.generate(1);
        ASSERT_EQ(rsb.size(), 4);
        EXPECT_EQ(rsb.longest_sequence(), 1);
        rsb.generate(2);
        ASSERT_EQ(rsb.size(), 8);
        EXPECT_EQ(rsb.longest_sequence(), 2);
        rsb.generate(4); // 8 + 2^3 + 2^4 = 32
        ASSERT_EQ(rsb.size(), 32);
        EXPECT_EQ(rsb.longest_sequence(), 4);

        std::set<size_t> hashes;
        for (size_t i = 0; i < 32; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.raw_id, i);
            auto [where, new_elem] = hashes.emplace(rs.hash);
            EXPECT_TRUE(new_elem);
        }
        EXPECT_EQ(hashes.size(), 32);

        for (size_t i = 0; i < 2; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.operators.size(), 0);
        }
        for (size_t i = 2; i < 4; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.operators.size(), 1);
        }
        for (size_t i = 4; i < 8; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.operators.size(), 2);
        }
        for (size_t i = 8; i < 16; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.operators.size(), 3);
        }
        for (size_t i = 16; i < 32; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.operators.size(), 4);
        }
    }
}