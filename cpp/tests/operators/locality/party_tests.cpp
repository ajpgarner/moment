/**
 * party_info_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/locality/party.h"
#include "operators/locality/locality_context.h"
#include "operators/operator_sequence.h"

#include <vector>

namespace NPATK::Tests {
    TEST(Operators_Locality_Party, Construct_Basic) {
        std::vector<Party> the_party;
        the_party.emplace_back(5, std::vector{Measurement{"a", 4}});
        LocalityContext context{std::move(the_party)};
        const Party& party = context.Parties[0];


        EXPECT_EQ(party.id(), 0);
        EXPECT_EQ(party.name, "F");
        EXPECT_EQ(party.size(), 3);

        auto iter = party.begin();
        ASSERT_NE(iter, party.end());
        EXPECT_EQ(*iter, 0);
        EXPECT_EQ((*iter), party[0]);
        ++iter;

        ASSERT_NE(iter, party.end());
        EXPECT_EQ(*iter, 1);
        EXPECT_EQ((*iter), party[1]);
        ++iter;

        ASSERT_NE(iter, party.end());
        EXPECT_EQ(*iter, 2);
        EXPECT_EQ((*iter), party[2]);
        ++iter;

        ASSERT_EQ(iter, party.end());
    }

    TEST(Operators_Locality_Party, OneMeasurement) {
        std::vector<Party> the_party;
        the_party.emplace_back(0, "A", std::vector{Measurement{"X", 4}});
        LocalityContext context{std::move(the_party)};
        const Party& alice = context.Parties[0];

        EXPECT_EQ(alice.id(), 0);
        EXPECT_EQ(alice.name, "A");
        ASSERT_EQ(alice.size(), 3);

        // Test IDs
        EXPECT_EQ(alice[0], 0);
        EXPECT_EQ(alice[1], 1);
        EXPECT_EQ(alice[2], 2);

        // Test exclusivity:
        EXPECT_TRUE(alice.mutually_exclusive(alice[0], alice[1]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[0], alice[2]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[1], alice[0]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[1], alice[2]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[2], alice[0]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[2], alice[1]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[0], alice[0]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[1], alice[1]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[2], alice[2]));
    }

    TEST(Operators_Locality_Party, TwoMeasurement) {
        std::vector<Party> the_party;
        the_party.emplace_back(0, "A", std::vector{Measurement{"X", 3}, Measurement{"Y", 3}});
        LocalityContext context{std::move(the_party)};
        const Party& alice = context.Parties[0];

        EXPECT_EQ(alice.id(), 0);
        EXPECT_EQ(alice.name, "A");
        ASSERT_EQ(alice.size(), 4);
        EXPECT_EQ(alice.Measurements.size(), 2);

        // Test IDs
        EXPECT_EQ(alice[0], 0);
        EXPECT_EQ(alice[1], 1);
        EXPECT_EQ(alice[2], 2);
        EXPECT_EQ(alice[3], 3);

        // Test exclusivity:
        EXPECT_TRUE(alice.mutually_exclusive(alice[0], alice[1]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[0], alice[2]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[0], alice[3]));

        EXPECT_TRUE(alice.mutually_exclusive(alice[1], alice[0]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[1], alice[2]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[1], alice[3]));

        EXPECT_TRUE(alice.mutually_exclusive(alice[2], alice[3]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[2], alice[0]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[2], alice[1]));

        EXPECT_TRUE(alice.mutually_exclusive(alice[3], alice[2]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[3], alice[0]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[3], alice[1]));

        EXPECT_FALSE(alice.mutually_exclusive(alice[0], alice[0]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[1], alice[1]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[2], alice[2]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[3], alice[3]));
    }

    TEST(Operators_Locality_Party, MakeList_FromInitializer) {
        auto party_list = Party::MakeList({1, 1, 1}, {4, 5, 6});
        ASSERT_EQ(party_list.size(), 3);
        LocalityContext context{std::move(party_list)};
        ASSERT_EQ(context.Parties.size(), 3);

        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        const auto& charlie = context.Parties[2];

        ASSERT_EQ(alice.size(), 3);
        for (size_t i = 0; i < 3; ++i) {
            EXPECT_EQ(alice[i], i);
        }

        ASSERT_EQ(bob.size(), 4);
        for (size_t i = 0; i < 4; ++i) {
            EXPECT_EQ(bob[i], i + 3);
        }
        
        ASSERT_EQ(charlie.size(), 5);
        for (size_t i = 0; i < 5; ++i) {
            EXPECT_EQ(charlie[i], i + 3 + 4);
        }
    }

    TEST(Operators_Locality_Party, MakeList_PartyOper) {
        auto party_list = Party::MakeList(2, 1, 4);
        ASSERT_EQ(party_list.size(), 2);
        LocalityContext context{std::move(party_list)};
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 3);
        EXPECT_EQ(alice[0], 0);
        EXPECT_EQ(alice[1], 1);
        EXPECT_EQ(alice[2], 2);

        ASSERT_EQ(bob.size(), 3);
        EXPECT_EQ(bob[0], 3);
        EXPECT_EQ(bob[1], 4);
        EXPECT_EQ(bob[2], 5);
    }

    TEST(Operators_Locality_Party, MakeList_PartyMmtOper) {
        auto party_list = Party::MakeList(2, 2, 3);
        ASSERT_EQ(party_list.size(), 2);
        LocalityContext context{std::move(party_list)};
        ASSERT_EQ(context.Parties.size(), 2);

        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 4);
        EXPECT_TRUE(alice.mutually_exclusive(alice[0], alice[1]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[1], alice[0]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[2], alice[3]));
        EXPECT_TRUE(alice.mutually_exclusive(alice[3], alice[2]));
        
        EXPECT_FALSE(alice.mutually_exclusive(alice[0], alice[2]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[0], alice[3]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[1], alice[2]));
        EXPECT_FALSE(alice.mutually_exclusive(alice[1], alice[3]));
        
        ASSERT_EQ(bob.size(), 4);
        EXPECT_TRUE(bob.mutually_exclusive(bob[0], bob[1]));
        EXPECT_TRUE(bob.mutually_exclusive(bob[1], bob[0]));
        EXPECT_TRUE(bob.mutually_exclusive(bob[2], bob[3]));
        EXPECT_TRUE(bob.mutually_exclusive(bob[3], bob[2]));

        EXPECT_FALSE(bob.mutually_exclusive(bob[0], bob[2]));
        EXPECT_FALSE(bob.mutually_exclusive(bob[0], bob[3]));
        EXPECT_FALSE(bob.mutually_exclusive(bob[1], bob[2]));
        EXPECT_FALSE(bob.mutually_exclusive(bob[1], bob[3]));
        
    }



    TEST(Operators_Locality_Party, MakeList_Vector23) {
        auto party_list = Party::MakeList({2, 1}, {2, 3, 2});
        ASSERT_EQ(party_list.size(), 2);
        LocalityContext context{std::move(party_list)};
        ASSERT_EQ(context.Parties.size(), 2);

        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 3);
        ASSERT_EQ(alice.Measurements.size(), 2);
        EXPECT_EQ(alice[0], 0);
        EXPECT_EQ(alice[1], 1);
        EXPECT_EQ(alice[2], 2);

        ASSERT_EQ(bob.size(), 1);
        ASSERT_EQ(bob.Measurements.size(), 1);
        EXPECT_EQ(bob[0], 3);
    }

}