/**
 * algebraic_system_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/algebraic/algebraic_matrix_system.h"
#include "operators/algebraic/algebraic_context.h"

#include "raw_sequence_comparer.h"

#include <set>

namespace NPATK::Tests {

    void RawSequenceComparer::find_and_compare(std::vector<oper_name_t> op_seq,
                                          symbol_name_t expected_symbol) const {
        HashedSequence seq{std::move(op_seq), this->context.the_hasher()};
        auto entryPtr = this->book.where(seq);
        ASSERT_NE(entryPtr, nullptr) << "seq = " << seq << "\n" << this->book;
        const auto& entry = *entryPtr;
        EXPECT_EQ(entry.hash(), seq.hash());
        EXPECT_EQ(entry.raw_id, expected_symbol);
        EXPECT_EQ(static_cast<HashedSequence>(entry), seq);
    }

    void RawSequenceComparer::find_and_compare_zero() const {
        auto entryPtr = this->book.where(0);
        ASSERT_NE(entryPtr, nullptr);
        EXPECT_TRUE(entryPtr->empty());
        EXPECT_TRUE(entryPtr->zero());
        EXPECT_EQ(entryPtr->raw_id, 0);
        EXPECT_EQ(entryPtr->size(), 0);
        EXPECT_EQ(entryPtr->hash(), 0);
        EXPECT_EQ(entryPtr->conjugate_hash, 0);


    }

    void RawSequenceComparer::find_and_compare_id() const {
        auto entryPtr = this->book.where(1);
        ASSERT_NE(entryPtr, nullptr);
        EXPECT_TRUE(entryPtr->empty());
        EXPECT_FALSE(entryPtr->zero());
        EXPECT_EQ(entryPtr->raw_id, 1);
        EXPECT_EQ(entryPtr->size(), 0);
        EXPECT_EQ(entryPtr->hash(), 1);
        EXPECT_EQ(entryPtr->conjugate_hash, 1);
    }



    TEST(RawSequence, RawSequenceBook) {
        AlgebraicContext theContext{2}; // 2 symbols...
        RawSequenceBook rsb{theContext};
        ASSERT_FALSE(rsb.commutative);

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
            auto [where, new_elem] = hashes.emplace(rs.hash());
            EXPECT_TRUE(new_elem);
        }
        EXPECT_EQ(hashes.size(), 32);

        for (size_t i = 0; i < 2; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.size(), 0);
        }
        for (size_t i = 2; i < 4; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.size(), 1);
        }
        for (size_t i = 4; i < 8; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.size(), 2);
        }
        for (size_t i = 8; i < 16; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.size(), 3);
        }
        for (size_t i = 16; i < 32; ++i) {
            const auto& rs = rsb[i];
            EXPECT_EQ(rs.size(), 4);
        }
    }



    TEST(RawSequence, RawSequenceBook_Commuting) {

        // Make book
        AlgebraicContext theContext{3, true, true}; // 3 Hermitian symbols that commute...
        const auto& hasher = theContext.the_hasher();
        RawSequenceBook rsb{theContext, true};
        ASSERT_TRUE(rsb.commutative);

        RawSequenceComparer comparer{theContext, rsb};

        ASSERT_EQ(rsb.size(), 2) << rsb; // [0, 1]
        comparer.find_and_compare_zero();
        comparer.find_and_compare_id();

        rsb.generate(1);
        ASSERT_EQ(rsb.size(), 5) << rsb; // 2 + 3
        EXPECT_EQ(rsb.longest_sequence(), 1);
        comparer.find_and_compare({0}, 2);
        comparer.find_and_compare({1}, 3);
        comparer.find_and_compare({2}, 4);

        rsb.generate(2);
        ASSERT_EQ(rsb.size(), 11) << rsb; // 2 + 3 + 6
        EXPECT_EQ(rsb.longest_sequence(), 2);
        comparer.find_and_compare({0, 0}, 5);
        comparer.find_and_compare({0, 1}, 6);
        comparer.find_and_compare({0, 2}, 7);
        comparer.find_and_compare({1, 1}, 8);
        comparer.find_and_compare({1, 2}, 9);
        comparer.find_and_compare({2, 2}, 10);

        rsb.generate(3); //
        ASSERT_EQ(rsb.size(), 21) << rsb; // 2 + 3 + 6 + 9
        EXPECT_EQ(rsb.longest_sequence(), 3);
        comparer.find_and_compare({0, 0, 0}, 11);
        comparer.find_and_compare({0, 0, 1}, 12);
        comparer.find_and_compare({0, 0, 2}, 13);
        comparer.find_and_compare({0, 1, 1}, 14);
        comparer.find_and_compare({0, 1, 2}, 15);
        comparer.find_and_compare({0, 2, 2}, 16);
        comparer.find_and_compare({1, 1, 1}, 17);
        comparer.find_and_compare({1, 1, 2}, 18);
        comparer.find_and_compare({1, 2, 2}, 19);
        comparer.find_and_compare({2, 2, 2}, 20);
    }
}