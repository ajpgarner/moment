/**
 * multi_operator_iterator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/locality/locality_context.h"
#include "operators/multi_operator_iterator.h"

namespace Moment::Tests {

    TEST(Operators_MultiOperatorIterator, Construct_NoLength) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};

        MultiOperatorIterator iter{collection, 0};
        EXPECT_EQ(iter, iter);
        EXPECT_EQ(iter, MultiOperatorIterator::end_of(collection, 0));
    }

    TEST(Operators_MultiOperatorIterator, Construct_LengthOne2x2) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};

        MultiOperatorIterator iter{collection, 1};
        auto iter_end = MultiOperatorIterator::end_of(collection, 1);
        EXPECT_EQ(iter, iter);
        ASSERT_NE(iter, iter_end);


        auto osA0 = *iter;
        ASSERT_FALSE(osA0.empty());
        ASSERT_EQ(osA0.size(), 1);
        EXPECT_EQ(osA0[0], collection.Parties[0][0]);
        auto osRawA0 = iter.raw();
        EXPECT_EQ(osRawA0[0], collection.Parties[0][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osA1 = *iter;
        ASSERT_FALSE(osA1.empty());
        ASSERT_EQ(osA1.size(), 1);
        EXPECT_EQ(osA1[0], collection.Parties[0][1]);
        auto osRawA1 = iter.raw();
        EXPECT_EQ(osRawA1[0], collection.Parties[0][1]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB0 = *iter;
        ASSERT_FALSE(osB0.empty());
        ASSERT_EQ(osB0.size(), 1);
        EXPECT_EQ(osB0[0], collection.Parties[1][0]);
        auto osRawB0 = iter.raw();
        EXPECT_EQ(osRawB0[0], collection.Parties[1][0]);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB1 = *iter;
        ASSERT_FALSE(osB1.empty());
        ASSERT_EQ(osB1.size(), 1);
        EXPECT_EQ(osB1[0], collection.Parties[1][1]);
        auto osRawB1 = iter.raw();
        EXPECT_EQ(osRawB1[0], collection.Parties[1][1]);

        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(Operators_MultiOperatorIterator, Construct_LengthTwo) {
        Context collection{2};

        MultiOperatorIterator iter{collection, 2};
        auto iter_end = MultiOperatorIterator::end_of(collection, 2);
        EXPECT_EQ(iter, iter);
        ASSERT_NE(iter, iter_end);


        auto osA0 = *iter;
        ASSERT_FALSE(osA0.empty());
        ASSERT_EQ(osA0.size(), 2);
        EXPECT_EQ(osA0[0], 0);
        EXPECT_EQ(osA0[1], 0);
        auto osRawA0 = iter.raw();
        EXPECT_EQ(osRawA0[0], 0);
        EXPECT_EQ(osRawA0[1], 0);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osA1 = *iter;
        ASSERT_FALSE(osA1.empty());
        ASSERT_EQ(osA1.size(), 2);
        EXPECT_EQ(osA1[0], 0);
        EXPECT_EQ(osA1[1], 1);
        auto osRawA1 = iter.raw();
        EXPECT_EQ(osRawA1[0], 0);
        EXPECT_EQ(osRawA1[1], 1);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB0 = *iter;
        ASSERT_FALSE(osB0.empty());
        ASSERT_EQ(osB0.size(), 2);
        EXPECT_EQ(osB0[0], 1);
        EXPECT_EQ(osB0[1], 0);
        auto osRawB0 = iter.raw();
        EXPECT_EQ(osRawB0[0], 1);
        EXPECT_EQ(osRawB0[1], 0);

        ++iter;
        ASSERT_NE(iter, iter_end);

        auto osB1 = *iter;
        ASSERT_FALSE(osB1.empty());
        ASSERT_EQ(osB1.size(), 2);
        EXPECT_EQ(osB1[0], 1);
        EXPECT_EQ(osB1[1], 1);
        auto osRawB1 = iter.raw();
        EXPECT_EQ(osRawB1[0], 1);
        EXPECT_EQ(osRawB1[1], 1);


        ++iter;
        ASSERT_EQ(iter, iter_end);
    }

    TEST(Operators_MultiOperatorIterator, Construct_LengthFour) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};

        MultiOperatorIterator iter{collection, 4};
        auto iter_end = MultiOperatorIterator::end_of(collection, 4);
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


    TEST(Operators_MultiOperatorIterator, RangeTest) {
        Context collection{4};
        ASSERT_EQ(collection.size(), 4);

        size_t count = 0;
        for (auto opStr : MultiOperatorRange{collection, 4}) {
            size_t v[4];
            v[0] = (count >> 6) & 0x3;
            v[1] = (count >> 4) & 0x3;
            v[2] = (count >> 2) & 0x3;
            v[3] = count & 0x3;

            ASSERT_EQ(opStr.size(), 4);
            EXPECT_EQ(opStr[0], v[0]) << "count = " << count << ", v[0]=" << v[0];
            EXPECT_EQ(opStr[1], v[1]) << "count = " << count << ", v[1]=" << v[1];
            EXPECT_EQ(opStr[2], v[2]) << "count = " << count << ", v[2]=" << v[2];
            EXPECT_EQ(opStr[3], v[3]) << "count = " << count << ", v[3]=" << v[3];
            ++count;
        }
        EXPECT_EQ(count, 256);
    }


}