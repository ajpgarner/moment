/**
 * operator_sequence_generator_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/multi_operator_iterator.h"
#include "operators/operator_sequence_generator.h"
#include "operators/locality/locality_context.h"

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


    TEST(Operators_OperatorSequenceGenerator, Empty) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};
        OperatorSequenceGenerator osg{collection, 0};
        compare_sequences(osg, {OperatorSequence::Identity(collection)});
    }

    TEST(Operators_OperatorSequenceGenerator, OneParty3Symbols1Length) {
        Context collection{3};
        std::vector<oper_name_t> alice{0, 1, 2};
        ASSERT_EQ(alice.size(), 3);

        OperatorSequenceGenerator osg{collection, 1};
        compare_sequences(osg, {OperatorSequence::Identity(collection),
                                OperatorSequence{{alice[0]}, collection},
                                OperatorSequence{{alice[1]}, collection},
                                OperatorSequence{{alice[2]}, collection}});
    }


    TEST(Operators_OperatorSequenceGenerator, TwoParty2Symbols2Length) {
        LocalityContext collection{Party::MakeList(2, 2, 2)};
        ASSERT_EQ(collection.Parties.size(), 2);
        const auto& alice = collection.Parties[0];
        const auto& bob = collection.Parties[1];
        ASSERT_EQ(alice.size(), 2);
        ASSERT_EQ(bob.size(), 2);

        OperatorSequenceGenerator osg{collection, 2};

        compare_sequences(osg, {OperatorSequence::Identity(collection),
                                OperatorSequence{{alice[0]}, collection},
                                OperatorSequence{{alice[1]}, collection},
                                OperatorSequence{{bob[0]}, collection},
                                OperatorSequence{{bob[1]}, collection},
                                OperatorSequence{{alice[0], alice[1]}, collection},
                                OperatorSequence{{alice[0], bob[0]}, collection},
                                OperatorSequence{{alice[0], bob[1]}, collection},
                                OperatorSequence{{alice[1], alice[0]}, collection},
                                OperatorSequence{{alice[1], bob[0]}, collection},
                                OperatorSequence{{alice[1], bob[1]}, collection},
                                OperatorSequence{{bob[0],   bob[1]}, collection},
                                OperatorSequence{{bob[1],  bob[0]}, collection}});
    }

    TEST(Operators_OperatorSequenceGenerator, OneParty3Symbols3Length_Mutex) {
        LocalityContext collection{Party::MakeList(1, 1, 4)};
        ASSERT_EQ(collection.Parties.size(), 1);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 3);
        ASSERT_TRUE(alice.mutually_exclusive(alice[0], alice[1]));
        ASSERT_TRUE(alice.mutually_exclusive(alice[0], alice[2]));
        ASSERT_TRUE(alice.mutually_exclusive(alice[1], alice[2]));

        OperatorSequenceGenerator osg{collection, 3};
        compare_sequences(osg, {OperatorSequence::Identity(collection),
                                OperatorSequence({alice[0]}, collection),
                                OperatorSequence({alice[1]}, collection),
                                OperatorSequence({alice[2]}, collection),
                                });
    }


    TEST(Operators_OperatorSequenceGenerator, TwoParty1Symbol_Idem) {
        LocalityContext collection{Party::MakeList(2, 1, 2)};
        ASSERT_EQ(collection.Parties.size(), 2);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        auto& bob = collection.Parties[1];
        ASSERT_EQ(bob.size(), 1);


        OperatorSequenceGenerator osg{collection, 2};
        compare_sequences(osg, {OperatorSequence::Identity(collection),
                                OperatorSequence({alice[0]}, collection),
                                OperatorSequence({bob[0]}, collection),
                                OperatorSequence({alice[0], bob[0]}, collection)});
    }

    TEST(Operators_OperatorSequenceGenerator, Conjugate_1Party2Symbols2Length) {
        Context collection{2};
        std::vector<oper_name_t> alice{0, 1};

        OperatorSequenceGenerator osg{collection, 2};
        ASSERT_EQ(osg.max_sequence_length, 2);
        auto osg_conj = osg.conjugate();
        compare_sequences(osg, {OperatorSequence::Identity(collection),
                                OperatorSequence({alice[0]}, collection),
                                OperatorSequence({alice[1]}, collection),
                                OperatorSequence({alice[0], alice[0]}, collection),
                                OperatorSequence({alice[0], alice[1]}, collection),
                                OperatorSequence({alice[1], alice[0]}, collection),
                                OperatorSequence({alice[1], alice[1]}, collection)});

        compare_sequences(osg_conj, {OperatorSequence::Identity(collection),
                                OperatorSequence({alice[0]}, collection),
                                OperatorSequence({alice[1]}, collection),
                                OperatorSequence({alice[0], alice[0]}, collection),
                                OperatorSequence({alice[1], alice[0]}, collection),
                                OperatorSequence({alice[0], alice[1]}, collection),
                                OperatorSequence({alice[1], alice[1]}, collection)});
    }

    TEST(Operators_OperatorSequenceGenerator, Conjugate_2Party1Symbols2Length) {
        LocalityContext collection{Party::MakeList(2, 1, 2)};
        ASSERT_EQ(collection.Parties.size(), 2);
        auto& alice = collection.Parties[0];
        ASSERT_EQ(alice.size(), 1);
        auto& bob = collection.Parties[1];
        ASSERT_EQ(bob.size(), 1);


        OperatorSequenceGenerator osg{collection, 2};
        auto osg_conj = osg.conjugate();
        EXPECT_EQ(osg_conj.max_sequence_length, osg.max_sequence_length);

        compare_sequences(osg, {OperatorSequence::Identity(collection),
                                OperatorSequence({alice[0]}, collection),
                                OperatorSequence({bob[0]}, collection),
                                OperatorSequence({alice[0], bob[0]}, collection)});

        compare_sequences(osg_conj, {OperatorSequence::Identity(collection),
                                    OperatorSequence({alice[0]}, collection),
                                    OperatorSequence({bob[0]}, collection),
                                    OperatorSequence({alice[0], bob[0]}, collection)});
    }
}