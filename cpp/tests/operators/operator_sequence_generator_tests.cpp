/**
 * operator_sequence_generator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/multi_operator_iterator.h"
#include "operators/operator_sequence_generator.h"

namespace NPATK::Tests {

    void compare_sequences(OperatorSequenceGenerator& osg, std::initializer_list<OperatorSequence> refSeq) {
        ASSERT_EQ(osg.size(), refSeq.size());

        auto iter = osg.begin();
        size_t index = 0;
        for (const auto& os : refSeq) {
            ASSERT_NE(iter, osg.end());
            EXPECT_EQ(&osg[index], &(*iter));
            EXPECT_EQ(osg[index], os);

            ASSERT_NE(iter, osg.end());
            ++index;
            ++iter;
        }
        EXPECT_EQ(iter, osg.end());
    }


    TEST(OperatorSequenceGenerator, Empty) {
        Context collection{2, 2};
        OperatorSequenceGenerator osg{collection, 0};
        compare_sequences(osg, {OperatorSequence::Identity(&collection)});
    }

    TEST(OperatorSequenceGenerator, OneParty3Symbols1Length) {
        Context collection{3};
        ASSERT_EQ(collection.Parties.size(), 1);
        const auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 3);

        OperatorSequenceGenerator osg{collection, 1};
        compare_sequences(osg, {OperatorSequence::Identity(&collection),
                                OperatorSequence{alice[0]},
                                OperatorSequence{alice[1]},
                                OperatorSequence{alice[2]}});
    }


    TEST(OperatorSequenceGenerator, TwoParty2Symbols2Length) {
        Context collection{2, 2};
        ASSERT_EQ(collection.Parties.size(), 2);
        const auto& alice = collection.Parties[0];
        const auto& bob = collection.Parties[1];
        ASSERT_EQ(alice.size(), 2);
        ASSERT_EQ(bob.size(), 2);

        OperatorSequenceGenerator osg{collection, 2};

        compare_sequences(osg, {OperatorSequence::Identity(&collection),
                                OperatorSequence{alice[0]},
                                OperatorSequence{alice[1]},
                                OperatorSequence{bob[0]},
                                OperatorSequence{bob[1]},
                                OperatorSequence{alice[0], alice[0]},
                                OperatorSequence{alice[0], alice[1]},
                                OperatorSequence{alice[0], bob[0]},
                                OperatorSequence{alice[0], bob[1]},
                                OperatorSequence{alice[1], alice[0]},
                                OperatorSequence{alice[1], alice[1]},
                                OperatorSequence{alice[1], bob[0]},
                                OperatorSequence{alice[1], bob[1]},
                                OperatorSequence{bob[0],   bob[0]},
                                OperatorSequence{bob[0],   bob[1]},
                                OperatorSequence{bob[1],  bob[0]},
                                OperatorSequence{bob[1],  bob[1]}});
    }

    TEST(OperatorSequenceGenerator, OneParty3Symbols3Length_Mutex) {
        Context collection{3};
        ASSERT_EQ(collection.Parties.size(), 1);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 3);
        alice.add_mutex(1, 2);
        ASSERT_FALSE(alice.exclusive(0, 1));
        ASSERT_FALSE(alice.exclusive(0, 2));
        ASSERT_TRUE(alice.exclusive(1, 2));

        OperatorSequenceGenerator osg{collection, 3};
        compare_sequences(osg, {OperatorSequence::Identity(&collection),
                                OperatorSequence({alice[0]}, &collection),
                                OperatorSequence({alice[1]}, &collection),
                                OperatorSequence({alice[2]}, &collection),
                                OperatorSequence({alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[0], alice[1]}, &collection),
                                OperatorSequence({alice[0], alice[2]}, &collection),
                                OperatorSequence({alice[1], alice[0]}, &collection),
                                OperatorSequence({alice[1], alice[1]}, &collection),
                                OperatorSequence({alice[2], alice[0]}, &collection),
                                OperatorSequence({alice[2], alice[2]}, &collection),
                                OperatorSequence({alice[0], alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[0], alice[0], alice[1]}, &collection),
                                OperatorSequence({alice[0], alice[0], alice[2]}, &collection),
                                OperatorSequence({alice[0], alice[1], alice[0]}, &collection),
                                OperatorSequence({alice[0], alice[1], alice[1]}, &collection),
                                OperatorSequence({alice[0], alice[2], alice[0]}, &collection),
                                OperatorSequence({alice[0], alice[2], alice[2]}, &collection),
                                OperatorSequence({alice[1], alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[1], alice[0], alice[1]}, &collection),
                                OperatorSequence({alice[1], alice[0], alice[2]}, &collection),
                                OperatorSequence({alice[1], alice[1], alice[0]}, &collection),
                                OperatorSequence({alice[1], alice[1], alice[1]}, &collection),
                                OperatorSequence({alice[2], alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[2], alice[0], alice[1]}, &collection),
                                OperatorSequence({alice[2], alice[0], alice[2]}, &collection),
                                OperatorSequence({alice[2], alice[2], alice[0]}, &collection),
                                OperatorSequence({alice[2], alice[2], alice[2]}, &collection)});
    }


    TEST(OperatorSequenceGenerator, TwoParty1Symbol_Idem) {
        Context collection(2, 1, Operator::Flags::Idempotent);
        ASSERT_EQ(collection.Parties.size(), 2);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        auto& bob = collection.Parties[1];
        ASSERT_EQ(bob.size(), 1);


        OperatorSequenceGenerator osg{collection, 2};
        compare_sequences(osg, {OperatorSequence::Identity(&collection),
                                OperatorSequence({alice[0]}, &collection),
                                OperatorSequence({bob[0]}, &collection),
                                OperatorSequence({alice[0], bob[0]}, &collection)});
    }

    TEST(OperatorSequenceGenerator, Conjugate_1Party2Symbols2Length) {
        Context collection{2};
        ASSERT_EQ(collection.Parties.size(), 1);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 2);

        OperatorSequenceGenerator osg{collection, 2};
        ASSERT_EQ(osg.sequence_length, 2);
        auto osg_conj = osg.conjugate();
        compare_sequences(osg, {OperatorSequence::Identity(&collection),
                                OperatorSequence({alice[0]}, &collection),
                                OperatorSequence({alice[1]}, &collection),
                                OperatorSequence({alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[0], alice[1]}, &collection),
                                OperatorSequence({alice[1], alice[0]}, &collection),
                                OperatorSequence({alice[1], alice[1]}, &collection)});

        compare_sequences(osg_conj, {OperatorSequence::Identity(&collection),
                                OperatorSequence({alice[0]}, &collection),
                                OperatorSequence({alice[1]}, &collection),
                                OperatorSequence({alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[1], alice[0]}, &collection),
                                OperatorSequence({alice[0], alice[1]}, &collection),
                                OperatorSequence({alice[1], alice[1]}, &collection)});
    }

    TEST(OperatorSequenceGenerator, Conjugate_2Party1Symbols2Length) {
        Context collection{1, 1};
        ASSERT_EQ(collection.Parties.size(), 2);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        auto& bob = collection.Parties[1];
        ASSERT_EQ(bob.size(), 1);


        OperatorSequenceGenerator osg{collection, 2};
        auto osg_conj = osg.conjugate();
        EXPECT_EQ(osg_conj.sequence_length, osg.sequence_length);

        compare_sequences(osg, {OperatorSequence::Identity(&collection),
                                OperatorSequence({alice[0]}, &collection),
                                OperatorSequence({bob[0]}, &collection),
                                OperatorSequence({alice[0], alice[0]}, &collection),
                                OperatorSequence({alice[0], bob[0]}, &collection),
                                OperatorSequence({bob[0], bob[0]}, &collection)});

        compare_sequences(osg_conj, {OperatorSequence::Identity(&collection),
                                    OperatorSequence({alice[0]}, &collection),
                                    OperatorSequence({bob[0]}, &collection),
                                    OperatorSequence({alice[0], alice[0]}, &collection),
                                    OperatorSequence({alice[0], bob[0]}, &collection),
                                    OperatorSequence({bob[0], bob[0]}, &collection)});
    }
}