/**
 * party_info_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/party_info.h"
#include "operators/operator_sequence.h"

namespace NPATK::Tests {
    TEST(PartyInfo, Construct_Basic) {
        PartyInfo party(5, 3);
        const PartyInfo& crParty{party};

        EXPECT_EQ(party.id, 5);
        EXPECT_EQ(party.name, "5");
        EXPECT_EQ(party.size(), 3);

        auto iter = party.begin();
        ASSERT_NE(iter, party.end());
        EXPECT_EQ(iter->id, 0);
        EXPECT_EQ(iter->party, party);
        EXPECT_EQ(&(*iter), &party[0]);
        EXPECT_EQ(&(*iter), &crParty[0]);
        ++iter;

        ASSERT_NE(iter, party.end());
        EXPECT_EQ(iter->id, 1);
        EXPECT_EQ(iter->party, party);
        EXPECT_EQ(&(*iter), &party[1]);
        EXPECT_EQ(&(*iter), &crParty[1]);
        ++iter;

        ASSERT_NE(iter, party.end());
        EXPECT_EQ(iter->id, 2);
        EXPECT_EQ(iter->party, party);
        EXPECT_EQ(&(*iter), &party[2]);
        EXPECT_EQ(&(*iter), &crParty[2]);
        ++iter;

        ASSERT_EQ(iter, party.end());
    }

    TEST(PartyInfo, PartyInfo_Mutex) {
        PartyInfo party(5, 3);

        EXPECT_EQ(party.id, 5);
        EXPECT_EQ(party.name, "5");
        ASSERT_EQ(party.size(), 3);

        party.add_mutex(1, 2);
        EXPECT_FALSE(party.exclusive(0, 0));
        EXPECT_FALSE(party.exclusive(0, 1));
        EXPECT_FALSE(party.exclusive(0, 2));
        EXPECT_FALSE(party.exclusive(1, 0));
        EXPECT_FALSE(party.exclusive(1, 1));
        EXPECT_TRUE(party.exclusive(1, 2));
        EXPECT_FALSE(party.exclusive(2, 0));
        EXPECT_TRUE(party.exclusive(2, 1));
        EXPECT_FALSE(party.exclusive(2, 2));
    }

    TEST(PartyInfo, OneMeasurement) {
        PartyInfo alice(0, "A");
        alice.add_measurement(Measurement{"X", 3});
        EXPECT_EQ(alice.id, 0);
        EXPECT_EQ(alice.name, "A");
        ASSERT_EQ(alice.size(), 3);

        // Test IDs and flags
        EXPECT_EQ(alice[0].id, 0);
        EXPECT_TRUE(alice[0].idempotent());
        EXPECT_EQ(alice[1].id, 1);
        EXPECT_TRUE(alice[1].idempotent());
        EXPECT_EQ(alice[2].id, 2);
        EXPECT_TRUE(alice[2].idempotent());

        // Test exclusivity:
        EXPECT_TRUE(alice.exclusive(0, 1));
        EXPECT_TRUE(alice.exclusive(0, 2));
        EXPECT_TRUE(alice.exclusive(1, 0));
        EXPECT_TRUE(alice.exclusive(1, 2));
        EXPECT_TRUE(alice.exclusive(2, 0));
        EXPECT_TRUE(alice.exclusive(2, 1));
        EXPECT_FALSE(alice.exclusive(0, 0));
        EXPECT_FALSE(alice.exclusive(1, 1));
        EXPECT_FALSE(alice.exclusive(2, 2));
    }

    TEST(PartyInfo, TwoMeasurement) {
        PartyInfo alice(0, "A");
        alice.add_measurement(Measurement{"X", 2});
        alice.add_measurement(Measurement{"Y", 2});
        EXPECT_EQ(alice.id, 0);
        EXPECT_EQ(alice.name, "A");
        ASSERT_EQ(alice.size(), 4);

        // Test IDs and flags
        EXPECT_EQ(alice[0].id, 0);
        EXPECT_TRUE(alice[0].idempotent());
        EXPECT_EQ(alice[1].id, 1);
        EXPECT_TRUE(alice[1].idempotent());
        EXPECT_EQ(alice[2].id, 2);
        EXPECT_TRUE(alice[2].idempotent());
        EXPECT_EQ(alice[3].id, 3);
        EXPECT_TRUE(alice[3].idempotent());

        // Test exclusivity:
        EXPECT_TRUE(alice.exclusive(0, 1));
        EXPECT_FALSE(alice.exclusive(0, 2));
        EXPECT_FALSE(alice.exclusive(0, 3));

        EXPECT_TRUE(alice.exclusive(1, 0));
        EXPECT_FALSE(alice.exclusive(1, 2));
        EXPECT_FALSE(alice.exclusive(1, 3));

        EXPECT_TRUE(alice.exclusive(2, 3));
        EXPECT_FALSE(alice.exclusive(2, 0));
        EXPECT_FALSE(alice.exclusive(2, 1));

        EXPECT_TRUE(alice.exclusive(3, 2));
        EXPECT_FALSE(alice.exclusive(3, 0));
        EXPECT_FALSE(alice.exclusive(3, 1));

        EXPECT_FALSE(alice.exclusive(0, 0));
        EXPECT_FALSE(alice.exclusive(1, 1));
        EXPECT_FALSE(alice.exclusive(2, 2));
        EXPECT_FALSE(alice.exclusive(3, 3));
    }

    TEST(PartyInfo, MakeList_FromInitializer) {
        auto party_list = PartyInfo::MakeList({3, 4, 5}, Operator::Flags::Idempotent);
        ASSERT_EQ(party_list.size(), 3);
        const auto& alice = party_list[0];
        const auto& bob = party_list[1];
        const auto& charlie = party_list[2];

        ASSERT_EQ(alice.size(), 3);
        for (size_t i = 0; i < 3; ++i) {
            EXPECT_EQ(alice[i].id, i);
            EXPECT_TRUE(alice[i].idempotent()) << "i = " << i;
        }

        ASSERT_EQ(bob.size(), 4);
        for (size_t i = 0; i < 4; ++i) {
            EXPECT_EQ(bob[i].id, i);
            EXPECT_TRUE(bob[i].idempotent()) << "i = " << i;
        }
        
        ASSERT_EQ(charlie.size(), 5);
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(charlie[i].id, i);
            EXPECT_TRUE(charlie[i].idempotent()) << "i = " << i;
        }
    }

    TEST(PartyInfo, MakeList_PartyOper) {
        auto party_list = PartyInfo::MakeList(2, 3);
        ASSERT_EQ(party_list.size(), 2);
        const auto& alice = party_list[0];
        const auto& bob = party_list[1];

        ASSERT_EQ(alice.size(), 3);
        EXPECT_EQ(alice[0].id, 0);
        EXPECT_EQ(alice[1].id, 1);
        EXPECT_EQ(alice[2].id, 2);

        ASSERT_EQ(bob.size(), 3);
        EXPECT_EQ(bob[0].id, 0);
        EXPECT_EQ(bob[1].id, 1);
        EXPECT_EQ(bob[2].id, 2);
    }

    TEST(PartyInfo, MakeList_PartyMmtOper) {
        auto party_list = PartyInfo::MakeList(2, 2, 2, true);
        ASSERT_EQ(party_list.size(), 2);
        const auto& alice = party_list[0];
        const auto& bob = party_list[1];

        ASSERT_EQ(alice.size(), 4);
        EXPECT_TRUE(alice.exclusive(0, 1));
        EXPECT_TRUE(alice.exclusive(1, 0));
        EXPECT_TRUE(alice.exclusive(2, 3));
        EXPECT_TRUE(alice.exclusive(3, 2));
        
        EXPECT_FALSE(alice.exclusive(0, 2));
        EXPECT_FALSE(alice.exclusive(0, 3));
        EXPECT_FALSE(alice.exclusive(1, 2));
        EXPECT_FALSE(alice.exclusive(1, 3));
        
        ASSERT_EQ(bob.size(), 4);
        EXPECT_TRUE(bob.exclusive(0, 1));
        EXPECT_TRUE(bob.exclusive(1, 0));
        EXPECT_TRUE(bob.exclusive(2, 3));
        EXPECT_TRUE(bob.exclusive(3, 2));

        EXPECT_FALSE(bob.exclusive(0, 2));
        EXPECT_FALSE(bob.exclusive(0, 3));
        EXPECT_FALSE(bob.exclusive(1, 2));
        EXPECT_FALSE(bob.exclusive(1, 3));
        
    }

}