/*
 * context_tests.cpp
 *
 * (c) 2022 Austrian Academy of Sciences.
 */

#include "gtest/gtest.h"

#include "operators/context.h"

namespace NPATK::Tests {

    TEST(Context, PartyInfo_Construct) {
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


    TEST(Context, PartyInfo_Mutex) {
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


    TEST(Context, Construct_Empty) {
        Context npa_gen(0, 0);
        ASSERT_EQ(npa_gen.Parties.size(), 0);
        ASSERT_TRUE(npa_gen.Parties.empty());

        auto iter_begin = npa_gen.begin();
        auto iter_end = npa_gen.end();
        EXPECT_EQ(iter_begin, iter_end);

        EXPECT_EQ(npa_gen.size(), 0);
    }

    TEST(Context, Construct_2x2) {
        Context npa_gen(2, 2);
        ASSERT_EQ(npa_gen.size(), 4);
        ASSERT_EQ(npa_gen.Parties.size(), 2);
        ASSERT_FALSE(npa_gen.Parties.empty());

        auto& alice = npa_gen.Parties[0];
        auto& bob = npa_gen.Parties[1];

        ASSERT_EQ(alice.size(), 2);
        ASSERT_FALSE(alice.empty());
        ASSERT_EQ(bob.size(), 2);
        ASSERT_FALSE(bob.empty());

        // Alice, 0
        auto all_iter = npa_gen.begin();
        ASSERT_NE(all_iter, npa_gen.end());
        auto alice_iter = alice.begin();
        ASSERT_NE(alice_iter, alice.end());
        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 0);

        // Alice, 1
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 0);

        // Bob, 0
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_EQ(alice_iter, alice.end());
        auto bob_iter = bob.begin();
        ASSERT_NE(bob_iter, bob.end());
        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 1);

        // Bob, 1
        ++all_iter; ++bob_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_NE(bob_iter, bob.end());

        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 1);

        // End
        ++all_iter; ++bob_iter;
        ASSERT_EQ(all_iter, npa_gen.end());
        ASSERT_EQ(bob_iter, bob.end());
    }

    TEST(Context, Construct_3_2) {
        Context npa_gen({3, 2});
        ASSERT_EQ(npa_gen.size(), 5);

        ASSERT_EQ(npa_gen.Parties.size(), 2);
        ASSERT_FALSE(npa_gen.Parties.empty());

        auto& alice = npa_gen.Parties[0];
        auto& bob = npa_gen.Parties[1];

        ASSERT_EQ(alice.size(), 3);
        ASSERT_FALSE(alice.empty());
        ASSERT_EQ(bob.size(), 2);
        ASSERT_FALSE(bob.empty());

        // Alice, 0
        auto all_iter = npa_gen.begin();
        ASSERT_NE(all_iter, npa_gen.end());
        auto alice_iter = alice.begin();
        ASSERT_NE(alice_iter, alice.end());
        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 0);

        // Alice, 1
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 0);

        // Alice, 2
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_NE(alice_iter, alice.end());

        EXPECT_EQ(*all_iter, *alice_iter);
        EXPECT_EQ(all_iter->id, 2);
        EXPECT_EQ(all_iter->party.id, 0);

        // Bob, 0
        ++all_iter; ++alice_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_EQ(alice_iter, alice.end());
        auto bob_iter = bob.begin();
        ASSERT_NE(bob_iter, bob.end());
        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 0);
        EXPECT_EQ(all_iter->party.id, 1);

        // Bob, 1
        ++all_iter; ++bob_iter;
        ASSERT_NE(all_iter, npa_gen.end());
        ASSERT_NE(bob_iter, bob.end());

        EXPECT_EQ(*all_iter, *bob_iter);
        EXPECT_EQ(all_iter->id, 1);
        EXPECT_EQ(all_iter->party.id, 1);

        // End
        ++all_iter; ++bob_iter;
        ASSERT_EQ(all_iter, npa_gen.end());
        ASSERT_EQ(bob_iter, bob.end());
    }

    TEST(Context, Construct_SpecDefaultFlags) {
        Context npa_gen(4, 3, Operator::Flags::Idempotent);
        ASSERT_EQ(npa_gen.size(), 12);

        size_t count = 0;
        for (auto& op : npa_gen) {
            ++count;
            EXPECT_EQ(op.flags,  Operator::Flags::Idempotent);
        }
        EXPECT_EQ(count, 12);
    }


    TEST(Context, Construct_ListDefaultFlags) {
        Context npa_gen({3, 2, 4}, Operator::Flags::Idempotent);
        ASSERT_EQ(npa_gen.size(), 9);

        size_t count = 0;
        for (auto& op : npa_gen) {
            ++count;
            EXPECT_EQ(op.flags,  Operator::Flags::Idempotent);
        }
        EXPECT_EQ(count, 9);
    }

    TEST(Context, Set_FlagWithin) {
        Context npa_gen({3, 2});
        ASSERT_EQ(npa_gen.size(), 5);
        ASSERT_EQ(npa_gen.Parties.size(), 2);
        auto& alice = npa_gen.Parties[0];
        auto& bob = npa_gen.Parties[1];

        ASSERT_EQ(alice.size(), 3);
        EXPECT_EQ(npa_gen.Parties[0][1].flags, Operator::Flags::None);
        EXPECT_EQ(alice[1].flags,  Operator::Flags::None);

        npa_gen.Parties[0][1].flags = Operator::Flags::Idempotent;

        EXPECT_EQ(npa_gen.Parties[0][1].flags, Operator::Flags::Idempotent);
        EXPECT_EQ(alice[1].flags,  Operator::Flags::Idempotent);
    }
}