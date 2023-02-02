/*
 * locality_context_tests.cpp
 *
 * (c) 2022 Austrian Academy of Sciences.
 */

#include "gtest/gtest.h"

#include "scenarios/operator_sequence.h"
#include "scenarios/locality/locality_context.h"

#include <optional>

namespace Moment::Tests {
    using namespace Moment::Locality;

    TEST(Scenarios_Locality_LocalityContext, Construct_Empty) {
        LocalityContext context{};
        ASSERT_EQ(context.Parties.size(), 0);
        ASSERT_TRUE(context.Parties.empty());

        EXPECT_EQ(context.size(), 0);
    }

    TEST(Scenarios_Locality_LocalityContext, Construct_2x2) {
        LocalityContext context(Party::MakeList(2, 2, 2));
        ASSERT_EQ(context.size(), 4);
        ASSERT_EQ(context.Parties.size(), 2);
        ASSERT_FALSE(context.Parties.empty());

        std::vector<oper_name_t> all_opers{0, 1, 2, 3};

        auto& alice = context.Parties[0];
        auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 2);
        ASSERT_FALSE(alice.empty());
        ASSERT_EQ(bob.size(), 2);
        ASSERT_FALSE(bob.empty());

        // Alice, 0
        auto all_iter = all_opers.begin();
        ASSERT_NE(all_iter, all_opers.end());
        auto alice_iter = alice.begin();
        ASSERT_NE(alice_iter, alice.end());
        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(*all_iter, 0);

        // Alice, 1
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(*all_iter, 1);

        // Bob, 0
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_EQ(alice_iter, alice.end());
        auto bob_iter = bob.begin();
        ASSERT_NE(bob_iter, bob.end());
        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(*all_iter, 2);

        // Bob, 1
        ++all_iter; ++bob_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_NE(bob_iter, bob.end());

        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(*all_iter, 3);

        // End
        ++all_iter; ++bob_iter;
        ASSERT_EQ(all_iter, all_opers.end());
        ASSERT_EQ(bob_iter, bob.end());
    }

    TEST(Scenarios_Locality_LocalityContext, Construct_3_2) {
        LocalityContext context(Party::MakeList({1,1}, {4, 3}));
        ASSERT_EQ(context.size(), 5);
        std::vector<oper_name_t> all_opers{0, 1, 2, 3, 4};

        ASSERT_EQ(context.Parties.size(), 2);
        ASSERT_FALSE(context.Parties.empty());

        auto& alice = context.Parties[0];
        auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 3);
        ASSERT_FALSE(alice.empty());
        ASSERT_EQ(bob.size(), 2);
        ASSERT_FALSE(bob.empty());

        // Alice, 0
        auto all_iter = all_opers.begin();
        ASSERT_NE(all_iter, all_opers.end());
        auto alice_iter = alice.begin();
        ASSERT_NE(alice_iter, alice.end());
        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(*all_iter, 0);

        // Alice, 1
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(*all_iter, 1);

        // Alice, 2
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(*all_iter, 2);

        // Bob, 0
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_EQ(alice_iter, alice.end());
        auto bob_iter = bob.begin();
        ASSERT_NE(bob_iter, bob.end());
        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(*all_iter, 3);

        // Bob, 1
        ++all_iter; ++bob_iter;
        ASSERT_NE(all_iter, all_opers.end());
        ASSERT_NE(bob_iter, bob.end());

        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(*all_iter, 4);

        // End
        ++all_iter; ++bob_iter;
        ASSERT_EQ(all_iter, all_opers.end());
        ASSERT_EQ(bob_iter, bob.end());
    }

    TEST(Scenarios_Locality_LocalityContext, Hash) {
        LocalityContext context(Party::MakeList(2, 2, 2));
        ASSERT_EQ(context.size(), 4);
        ASSERT_EQ(context.Parties.size(), 2);
        auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);
        auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 2);

        std::set<size_t> hashes{};

        auto a0 = context.hash(OperatorSequence{{alice[0]}, context});
        auto a0a1 = context.hash(OperatorSequence{{alice[0], alice[1]}, context});
        EXPECT_NE(a0a1, a0);
        auto a0a1a0 = context.hash(OperatorSequence{{alice[0], alice[1], alice[0]}, context});
        EXPECT_NE(a0a1a0, a0);
        EXPECT_NE(a0a1a0, a0a1);
        hashes.insert(a0);
        hashes.insert(a0a1);
        hashes.insert(a0a1a0);

        auto b0 = context.hash(OperatorSequence{{bob[0]}, context});
        EXPECT_FALSE(hashes.contains(b0));
        hashes.insert(b0);

        auto a0b0 = context.hash(OperatorSequence{{alice[0], bob[0]}, context});
        EXPECT_FALSE(hashes.contains(a0b0));
        hashes.insert(a0b0);

        auto a0a1b0 = context.hash(OperatorSequence{{alice[0], alice[1], bob[0]}, context});
        EXPECT_FALSE(hashes.contains(a0a1b0));
        hashes.insert(a0a1b0);

        auto b0a0a1 = context.hash(OperatorSequence{{bob[0], alice[0], alice[1]}, context});
        EXPECT_TRUE(hashes.contains(b0a0a1));

        auto a1 = context.hash(OperatorSequence{{alice[1]}, context});
        EXPECT_FALSE(hashes.contains(a1));
        hashes.insert(a1);

        auto b1 = context.hash(OperatorSequence{{bob[1]}, context});
        EXPECT_FALSE(hashes.contains(b1));
        hashes.insert(b1);
    }

    TEST(Scenarios_Locality_LocalityContext, Hash_Zero) {
        LocalityContext context(Party::MakeList(2, 2, 2));
        OperatorSequence zero{OperatorSequence::Zero(context)};
        ASSERT_TRUE(zero.zero());

        size_t hash = context.hash(zero);
        EXPECT_EQ(hash, 0);
    }

    TEST(Scenarios_Locality_LocalityContext, Hash_Identity) {
        LocalityContext context(Party::MakeList(2, 2, 2));
        OperatorSequence identity{OperatorSequence::Identity(context)};
        ASSERT_FALSE(identity.zero());

        size_t hash = context.hash(identity);
        EXPECT_EQ(hash, 1);
    }

    TEST(Scenarios_Locality_LocalityContext, EnumerateMeasurements) {
        LocalityContext context(Party::MakeList(2, 2, 2));
        ASSERT_EQ(context.Parties.size(), 2);
        const auto& alice = context.Parties[0];
        const auto& bob = context.Parties[1];
        ASSERT_EQ(alice.Measurements.size(), 2);
        ASSERT_EQ(bob.Measurements.size(), 2);

        const auto& alice_a = alice.Measurements[0];
        const auto& alice_b = alice.Measurements[1];
        const auto& bob_a = bob.Measurements[0];
        const auto& bob_b = bob.Measurements[1];

        EXPECT_EQ(alice_a.Index().global_mmt, 0);
        EXPECT_EQ(alice_a.Index().party, 0);
        EXPECT_EQ(alice_a.Index().mmt, 0);

        EXPECT_EQ(alice_b.Index().global_mmt, 1);
        EXPECT_EQ(alice_b.Index().party, 0);
        EXPECT_EQ(alice_b.Index().mmt, 1);

        EXPECT_EQ(bob_a.Index().global_mmt, 2);
        EXPECT_EQ(bob_a.Index().party, 1);
        EXPECT_EQ(bob_a.Index().mmt, 0);

        EXPECT_EQ(bob_b.Index().global_mmt, 3);
        EXPECT_EQ(bob_b.Index().party, 1);
        EXPECT_EQ(bob_b.Index().mmt, 1);
    }


    TEST(Scenarios_Locality_LocalityContext, PMToGlobalIndex) {
        LocalityContext context(Party::MakeList(2, 2, 2));

        const std::vector<PMIndex> trial{{0, 0}, {0, 1}, {1, 0}, {1, 1}};
        const std::vector<size_t> expected{0, 1, 2, 3};
        EXPECT_EQ(context.PM_to_global_index(trial), expected);
    }
}