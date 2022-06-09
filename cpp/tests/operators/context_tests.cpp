/*
 * context_tests.cpp
 *
 * (c) 2022 Austrian Academy of Sciences.
 */

#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/operator_sequence.h"

namespace NPATK::Tests {
    TEST(Context, Construct_Empty) {
        Context context(PartyInfo::MakeList(0, 0));
        ASSERT_EQ(context.Parties.size(), 0);
        ASSERT_TRUE(context.Parties.empty());

        auto iter_begin = context.begin();
        auto iter_end = context.end();
        EXPECT_EQ(iter_begin, iter_end);

        EXPECT_EQ(context.size(), 0);
    }

    TEST(Context, Construct_2x2) {
        Context context(PartyInfo::MakeList(2, 2));
        ASSERT_EQ(context.size(), 4);
        ASSERT_EQ(context.Parties.size(), 2);
        ASSERT_FALSE(context.Parties.empty());

        auto& alice = context.Parties[0];
        auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 2);
        ASSERT_FALSE(alice.empty());
        ASSERT_EQ(bob.size(), 2);
        ASSERT_FALSE(bob.empty());

        // Alice, 0
        auto all_iter = context.begin();
        ASSERT_NE(all_iter, context.end());
        auto alice_iter = alice.begin();
        ASSERT_NE(alice_iter, alice.end());
        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 0);

        // Alice, 1
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 0);

        // Bob, 0
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_EQ(alice_iter, alice.end());
        auto bob_iter = bob.begin();
        ASSERT_NE(bob_iter, bob.end());
        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 1);

        // Bob, 1
        ++all_iter; ++bob_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_NE(bob_iter, bob.end());

        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 1);

        // End
        ++all_iter; ++bob_iter;
        ASSERT_EQ(all_iter, context.end());
        ASSERT_EQ(bob_iter, bob.end());
    }

    TEST(Context, Construct_3_2) {
        Context context(PartyInfo::MakeList({3, 2}));
        ASSERT_EQ(context.size(), 5);

        ASSERT_EQ(context.Parties.size(), 2);
        ASSERT_FALSE(context.Parties.empty());

        auto& alice = context.Parties[0];
        auto& bob = context.Parties[1];

        ASSERT_EQ(alice.size(), 3);
        ASSERT_FALSE(alice.empty());
        ASSERT_EQ(bob.size(), 2);
        ASSERT_FALSE(bob.empty());

        // Alice, 0
        auto all_iter = context.begin();
        ASSERT_NE(all_iter, context.end());
        auto alice_iter = alice.begin();
        ASSERT_NE(alice_iter, alice.end());
        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 0);

        // Alice, 1
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 0);

        // Alice, 2
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 2);
        EXPECT_EQ(all_iter->party.id, 0);

        // Bob, 0
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_EQ(alice_iter, alice.end());
        auto bob_iter = bob.begin();
        ASSERT_NE(bob_iter, bob.end());
        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 1);

        // Bob, 1
        ++all_iter; ++bob_iter;
        ASSERT_NE(all_iter, context.end());
        ASSERT_NE(bob_iter, bob.end());

        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 1);

        // End
        ++all_iter; ++bob_iter;
        ASSERT_EQ(all_iter, context.end());
        ASSERT_EQ(bob_iter, bob.end());
    }

    TEST(Context, Construct_SpecDefaultFlags) {
        Context context(PartyInfo::MakeList(4, 3, Operator::Flags::Idempotent));
        ASSERT_EQ(context.size(), 12);

        size_t count = 0;
        for (auto& op : context) {
            ++count;
            EXPECT_EQ(op.flags,  Operator::Flags::Idempotent);
        }
        EXPECT_EQ(count, 12);
    }


    TEST(Context, Construct_ListDefaultFlags) {
        Context context(PartyInfo::MakeList({3, 2, 4}, Operator::Flags::Idempotent));
        ASSERT_EQ(context.size(), 9);

        size_t count = 0;
        for (auto& op : context) {
            ++count;
            EXPECT_EQ(op.flags,  Operator::Flags::Idempotent);
        }
        EXPECT_EQ(count, 9);
    }

    TEST(Context, AddParty) {
        Context context{};
        ASSERT_EQ(context.Parties.size(), 0);
        ASSERT_TRUE(context.Parties.empty());
        ASSERT_EQ(context.size(), 0);

        context.add_party(PartyInfo{0, "A", 3});
        ASSERT_EQ(context.Parties.size(), 1);
        EXPECT_EQ(context.size(), 3);

        context.add_party(PartyInfo{0, "B", 4});
        ASSERT_EQ(context.Parties.size(), 2);
        EXPECT_EQ(context.size(), 7);


    }


    TEST(Context, Hash) {
        Context context({2, 2});
        ASSERT_EQ(context.size(), 4);
        ASSERT_EQ(context.Parties.size(), 2);
        auto& alice = context.Parties[0];
        ASSERT_EQ(alice.size(), 2);
        auto& bob = context.Parties[1];
        ASSERT_EQ(bob.size(), 2);

        std::set<size_t> hashes{};

        auto a0 = context.hash(OperatorSequence{alice[0]});
        auto a0a0 = context.hash(OperatorSequence{alice[0], alice[0]});
        EXPECT_NE(a0a0, a0);
        auto a0a0a0 = context.hash(OperatorSequence{alice[0], alice[0], alice[0]});
        EXPECT_NE(a0a0a0, a0);
        EXPECT_NE(a0a0a0, a0a0);
        hashes.insert(a0);
        hashes.insert(a0a0);
        hashes.insert(a0a0a0);

        auto b0 = context.hash(OperatorSequence{bob[0]});
        EXPECT_FALSE(hashes.contains(b0));
        hashes.insert(b0);

        auto a0b0 = context.hash(OperatorSequence{alice[0], bob[0]});
        EXPECT_FALSE(hashes.contains(a0b0));
        hashes.insert(a0b0);

        auto a0a0b0 = context.hash(OperatorSequence{alice[0], alice[0], bob[0]});
        EXPECT_FALSE(hashes.contains(a0a0b0));
        hashes.insert(a0a0b0);

        auto b0a0a0 = context.hash(OperatorSequence{bob[0], alice[0], alice[0]});
        EXPECT_TRUE(hashes.contains(b0a0a0));

        auto a1 = context.hash(OperatorSequence{alice[1]});
        EXPECT_FALSE(hashes.contains(a1));
        hashes.insert(a1);

        auto b1 = context.hash(OperatorSequence{bob[1]});
        EXPECT_FALSE(hashes.contains(b1));
        hashes.insert(b1);
    }

    TEST(Context, Hash_Zero) {
        Context context({2, 2});
        OperatorSequence zero{OperatorSequence::Zero(&context)};
        ASSERT_TRUE(zero.zero());

        size_t hash = context.hash(zero);
        EXPECT_EQ(hash, 0);
    }

    TEST(Context, Hash_Identity) {
        Context context({2, 2});
        OperatorSequence identity{OperatorSequence::Identity(&context)};
        ASSERT_FALSE(identity.zero());

        size_t hash = context.hash(identity);
        EXPECT_EQ(hash, 1);
    }


}