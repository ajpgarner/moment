/**
 * multi_operator_iterator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/multi_operator_iterator.h"

namespace NPATK::Tests {

    TEST(MultiOperatorIterator, Construct_NoLength) {
        OperatorCollection collection{2, 2};

        detail::MultiOperatorIterator iter{collection, 0};
        EXPECT_EQ(iter, iter);
        EXPECT_EQ(iter, detail::MultiOperatorIterator::end_of(collection, 0));
    }

    TEST(MultiOperatorIterator, Construct_LengthOne2x2) {
        OperatorCollection collection{2, 2};

        detail::MultiOperatorIterator iter{collection, 1};
        auto iter_end = detail::MultiOperatorIterator::end_of(collection, 1);
        EXPECT_EQ(iter, iter);
        ASSERT_NE(iter, iter_end);


        auto osA0 = *iter;
        ASSERT_FALSE(osA0.empty());
        ASSERT_EQ(osA0.size(), 1);
        EXPECT_EQ(osA0[0], collection.Parties[0][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osA1 = *iter;
        ASSERT_FALSE(osA1.empty());
        ASSERT_EQ(osA1.size(), 1);
        EXPECT_EQ(osA1[0], collection.Parties[0][1]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB0 = *iter;
        ASSERT_FALSE(osB0.empty());
        ASSERT_EQ(osB0.size(), 1);
        EXPECT_EQ(osB0[0], collection.Parties[1][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB1 = *iter;
        ASSERT_FALSE(osB1.empty());
        ASSERT_EQ(osB1.size(), 1);
        EXPECT_EQ(osB1[0], collection.Parties[1][1]);

        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(MultiOperatorIterator, Construct_LengthTwo) {
        OperatorCollection collection{2};

        detail::MultiOperatorIterator iter{collection, 2};
        auto iter_end = detail::MultiOperatorIterator::end_of(collection, 2);
        EXPECT_EQ(iter, iter);
        ASSERT_NE(iter, iter_end);


        auto osA0 = *iter;
        ASSERT_FALSE(osA0.empty());
        ASSERT_EQ(osA0.size(), 2);
        EXPECT_EQ(osA0[0], collection.Parties[0][0]);
        EXPECT_EQ(osA0[1], collection.Parties[0][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osA1 = *iter;
        ASSERT_FALSE(osA1.empty());
        ASSERT_EQ(osA1.size(), 2);
        EXPECT_EQ(osA1[0], collection.Parties[0][0]);
        EXPECT_EQ(osA1[1], collection.Parties[0][1]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB0 = *iter;
        ASSERT_FALSE(osB0.empty());
        ASSERT_EQ(osB0.size(), 2);
        EXPECT_EQ(osB0[0], collection.Parties[0][1]);
        EXPECT_EQ(osB0[1], collection.Parties[0][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB1 = *iter;
        ASSERT_FALSE(osB1.empty());
        ASSERT_EQ(osB1.size(), 2);
        EXPECT_EQ(osB1[0], collection.Parties[0][1]);
        EXPECT_EQ(osB1[1], collection.Parties[0][1]);

        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(MultiOperatorIterator, Construct_LengthFour) {
        OperatorCollection collection{2, 2};

        detail::MultiOperatorIterator iter{collection, 4};
        auto iter_end = detail::MultiOperatorIterator::end_of(collection, 4);
        // 4 * 4 * 4 * 4 = 256 combos
        for (size_t count = 0; count < 256; ++count) {
            ASSERT_NE(iter, iter_end);
            auto op_seq = *iter;
            EXPECT_FALSE(op_seq.empty());
            EXPECT_LE(op_seq.size(), 4);
            ++iter;
        }
        EXPECT_EQ(iter, iter_end);
    }


    TEST(MultiOperatorIterator, RangeTest) {
        OperatorCollection collection{4};
        ASSERT_EQ(collection.Parties.size(), 1);
        const auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 4);

        size_t count = 0;
        for (auto opStr : detail::MultiOperatorRange{collection, 4}) {
            size_t v[4];
            v[0] = (count >> 6) & 0x3;
            v[1] = (count >> 4) & 0x3;
            v[2] = (count >> 2) & 0x3;
            v[3] = count & 0x3;

            ASSERT_EQ(opStr.size(), 4);
            EXPECT_EQ(opStr[0], alice[v[0]]) << "count = " << count << ", v[0]=" << v[0];
            EXPECT_EQ(opStr[1], alice[v[1]]) << "count = " << count << ", v[1]=" << v[1];
            EXPECT_EQ(opStr[2], alice[v[2]]) << "count = " << count << ", v[2]=" << v[2];
            EXPECT_EQ(opStr[3], alice[v[3]]) << "count = " << count << ", v[3]=" << v[3];
            ++count;
        }
        EXPECT_EQ(count, 256);
    }


}