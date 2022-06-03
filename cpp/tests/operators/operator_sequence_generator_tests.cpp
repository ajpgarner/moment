/**
 * operator_sequence_generator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/multi_operator_iterator.h"
#include "operators/operator_sequence_generator.h"

namespace NPATK::Tests {

    TEST(OperatorSequenceGenerator, Empty) {
        OperatorCollection collection{2, 2};
        OperatorSequenceGenerator osg{collection, 0};
        EXPECT_TRUE(osg.empty());
        EXPECT_EQ(osg.size(), 0);
        EXPECT_EQ(osg.begin(), osg.end());
    }

    TEST(OperatorSequenceGenerator, OneParty3Symbols1Length) {
        OperatorCollection collection{3};
        OperatorSequenceGenerator osg{collection, 1};
        ASSERT_FALSE(osg.empty());
        ASSERT_EQ(osg.size(), 3);

        auto iter = osg.begin();
        ASSERT_NE(iter, osg.end());

        auto seq_0 = *iter;
        OperatorSequence exp_0{collection.Parties[0][0]};
        EXPECT_EQ(seq_0, exp_0);
        EXPECT_EQ(osg[0], exp_0);

        ++iter;
        ASSERT_NE(iter, osg.end());
        auto seq_1 = *iter;
        OperatorSequence exp_1{collection.Parties[0][1]};
        EXPECT_EQ(seq_1, exp_1);
        EXPECT_EQ(osg[1], exp_1);

        ++iter;
        ASSERT_NE(iter, osg.end());
        auto seq_2 = *iter;
        OperatorSequence exp_2{collection.Parties[0][2]};
        EXPECT_EQ(seq_2, exp_2);
        EXPECT_EQ(osg[2], exp_2);

        ++iter;
        ASSERT_EQ(iter, osg.end());
    }

    TEST(OperatorSequenceGenerator, OneParty4Symbols4Length) {
        OperatorCollection collection{4};
        ASSERT_EQ(collection.Parties.size(), 1);
        const auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 4);


        size_t count = 0;
        OperatorSequenceGenerator osg{collection, 4};
        ASSERT_FALSE(osg.empty());
        ASSERT_EQ(osg.size(), 256);
        auto iter = osg.begin();

        for (auto opStr : detail::MultiOperatorRange{collection, 4}) {
            ASSERT_NE(iter, osg.end()) << "count = " << count;
            ASSERT_EQ(*iter, osg[count]) << "count = " << count;
            ASSERT_EQ(*iter, opStr) << "count = " << count;

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

            ++count; ++iter;
        }
        EXPECT_EQ(count, 256);
    }


    TEST(OperatorSequenceGenerator, TwoParty2Symbols2Length) {
        OperatorCollection collection{2, 2};
        const auto& alice = collection.Parties[0];
        const auto& bob = collection.Parties[1];
        ASSERT_EQ(alice.size(), 2);
        ASSERT_EQ(bob.size(), 2);

        OperatorSequenceGenerator osg{collection, 2};
        ASSERT_FALSE(osg.empty());
        ASSERT_EQ(osg.size(), 12); // 4 redundant strings removed!

        EXPECT_EQ(osg[0], (OperatorSequence{alice[0], alice[0]}));
        EXPECT_EQ(osg[1], (OperatorSequence{alice[0], alice[1]}));
        EXPECT_EQ(osg[2], (OperatorSequence{alice[0], bob[0]}));
        EXPECT_EQ(osg[3], (OperatorSequence{alice[0], bob[1]}));
        EXPECT_EQ(osg[4], (OperatorSequence{alice[1], alice[0]}));
        EXPECT_EQ(osg[5], (OperatorSequence{alice[1], alice[1]}));
        EXPECT_EQ(osg[6], (OperatorSequence{alice[1], bob[0]}));
        EXPECT_EQ(osg[7], (OperatorSequence{alice[1], bob[1]}));
        EXPECT_EQ(osg[8], (OperatorSequence{bob[0],   bob[0]}));
        EXPECT_EQ(osg[9], (OperatorSequence{bob[0],   bob[1]}));
        EXPECT_EQ(osg[10], (OperatorSequence{bob[1],  bob[0]}));
        EXPECT_EQ(osg[11], (OperatorSequence{bob[1],  bob[1]}));
    }
}