/**
 * locality_osg_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "dictionary/operator_sequence.h"
#include "scenarios/locality/locality_context.h"
#include "scenarios/locality/locality_osg.h"

#include <optional>

namespace Moment::Tests {
    using namespace Moment::Locality;

    namespace {
        void assert_is_id(const OperatorSequence &os, const Context &context) {
            EXPECT_TRUE(os.is_same_context(context));
            EXPECT_EQ(os.size(), 0);
            EXPECT_FALSE(os.zero());
        }

        template<typename iter_t>
        void assert_sequence(iter_t iter, const iter_t iter_end,
                             std::initializer_list<OperatorSequence> reference_list) {
            size_t done = 0;
            for (const auto& ref_seq : reference_list) {
                ASSERT_NE(iter, iter_end) << "Index = " << done;
                EXPECT_EQ(*iter, ref_seq) << "Index = " << done;
                ++done;
                ++iter;
            }
            EXPECT_EQ(iter, iter_end) << "Index = " << done;
        }

    }

    TEST(Scenarios_Locality_LocalityOSG, Empty_Length0) {
        LocalityContext context{};
        LocalityOperatorSequenceGenerator osg{context, 0};
        EXPECT_EQ(osg.PartyCount(), 0);
        ASSERT_EQ(osg.size(), 1); // id only

        assert_is_id(*osg.begin(), context);

    }
    TEST(Scenarios_Locality_LocalityOSG, Empty_Length1) {
        LocalityContext context{};
        LocalityOperatorSequenceGenerator osg{context, 1};
        EXPECT_EQ(osg.PartyCount(), 0);
        ASSERT_EQ(osg.size(), 1); // id only

        assert_is_id(*osg.begin(), context);
    }
    TEST(Scenarios_Locality_LocalityOSG, Empty_Length2) {
        LocalityContext context{};
        LocalityOperatorSequenceGenerator osg{context, 2};
        EXPECT_EQ(osg.PartyCount(), 0);

        ASSERT_EQ(osg.size(), 1); // id only

        assert_is_id(*osg.begin(), context);
    }

    TEST(Scenarios_Locality_LocalityOSG, OnePartyTwoOps_Length0) {
        LocalityContext context{Party::MakeList(1, 2, 2)};
        LocalityOperatorSequenceGenerator osg{context, 0};

        ASSERT_EQ(osg.PartyCount(), 1);

        // Check Party A:
        EXPECT_EQ(osg.Party(0).party, &(context.Parties[0]));
        ASSERT_EQ(osg.Party(0).word_length(), 0);
        auto alice0 = osg.Party(0)[0];
        ASSERT_EQ(alice0.size(), 1);
        assert_is_id(alice0[0], context);

        // Check whole sequence:
        ASSERT_EQ(osg.size(), 1); // id only

        assert_is_id(*osg.begin(), context);
    }

    TEST(Scenarios_Locality_LocalityOSG, OnePartyTwoOps_Length1) {
        LocalityContext context{Party::MakeList(1, 2, 2)};
        const auto a0 = context.Parties[0].global_offset();
        const auto a1 = static_cast<oper_name_t>(context.Parties[0].global_offset()+1);

        LocalityOperatorSequenceGenerator osg{context, 1};

        ASSERT_EQ(osg.PartyCount(), 1);

        // Check Party A:
        EXPECT_EQ(osg.Party(0).party, &(context.Parties[0]));
        ASSERT_EQ(osg.Party(0).word_length(), 1);
        auto alice0 = osg.Party(0)[0];
        ASSERT_EQ(alice0.size(), 1); // id
        assert_is_id(alice0[0], context);

        auto alice1 = osg.Party(0)[1];
        ASSERT_EQ(alice1.size(), 2); // a0, a1
        EXPECT_EQ(alice1[0], OperatorSequence({a0}, context));
        EXPECT_EQ(alice1[1], OperatorSequence({a1}, context));


        // Check whole sequence:
        ASSERT_EQ(osg.size(), 3); // id, a0, a1

        // Check OSG matches Alice
        auto all_alice = osg.Party(0).all();
        ASSERT_EQ(all_alice.size(), osg.size());

        auto iter = osg.begin();
        for (size_t idx = 0; idx < osg.size(); ++idx) {
            const auto& osg_seq = *iter;
            EXPECT_EQ(osg_seq, all_alice[idx]) << "idx = " << idx;
            ++iter;
        }
        EXPECT_EQ(iter, osg.end());



    }

    TEST(Scenarios_Locality_LocalityOSG, OnePartyTwoOps_Length2) {
        LocalityContext context{Party::MakeList(1, 2, 2)};
        const auto a0 = context.Parties[0].global_offset();
        const auto a1 = static_cast<oper_name_t>(context.Parties[0].global_offset()+1);

        LocalityOperatorSequenceGenerator osg{context, 2};

        ASSERT_EQ(osg.PartyCount(), 1);

        // Check Party A:
        EXPECT_EQ(osg.Party(0).party, &(context.Parties[0]));
        ASSERT_EQ(osg.Party(0).word_length(), 2);
        auto alice0 = osg.Party(0)[0];
        ASSERT_EQ(alice0.size(), 1);
        assert_is_id(alice0[0], context);

        auto alice1 = osg.Party(0)[1];
        ASSERT_EQ(alice1.size(), 2); // a0, a1
        EXPECT_EQ(alice1[0], OperatorSequence({a0}, context));
        EXPECT_EQ(alice1[1], OperatorSequence({a1}, context));


        auto alice2 = osg.Party(0)[2];
        ASSERT_EQ(alice2.size(), 2); // a0a1, a1a0
        EXPECT_EQ(alice2[0], OperatorSequence({a0, a1}, context));
        EXPECT_EQ(alice2[1], OperatorSequence({a1, a0}, context));

        ASSERT_EQ(osg.size(), 5); // id, a0, a1, a0a1, a1a0. [nb. a0a0 = a0, a1a1 = a1].

        // Check OSG matches Alice
        auto all_alice = osg.Party(0).all();
        ASSERT_EQ(all_alice.size(), osg.size());

        auto iter = osg.begin();
        for (size_t idx = 0; idx < osg.size(); ++idx) {
            const auto& osg_seq = *iter;
            EXPECT_EQ(osg_seq, all_alice[idx]) << "idx = " << idx;
            ++iter;
        }
        EXPECT_EQ(iter, osg.end());
    }


    TEST(Scenarios_Locality_LocalityOSG, CHSH_Length0) {
        LocalityContext context{Party::MakeList(2, 2, 2)};
        LocalityOperatorSequenceGenerator osg{context, 0};

        ASSERT_EQ(osg.PartyCount(), 2);

        // Check Party A:
        EXPECT_EQ(osg.Party(0).party, &(context.Parties[0]));
        ASSERT_EQ(osg.Party(0).word_length(), 0);
        auto alice0 = osg.Party(0)[0];
        ASSERT_EQ(alice0.size(), 1);
        assert_is_id(alice0[0], context);

        // Check Party B:
        EXPECT_EQ(osg.Party(1).party, &(context.Parties[1]));
        ASSERT_EQ(osg.Party(1).word_length(), 0);
        auto bob0 = osg.Party(1)[0];
        ASSERT_EQ(bob0.size(), 1);
        assert_is_id(bob0[0], context);

        // Check whole sequence:
        ASSERT_EQ(osg.size(), 1); // id only

        assert_is_id(*osg.begin(), context);
    }
    
    TEST(Scenarios_Locality_LocalityOSG, CHSH_Length1) {
        LocalityContext context{Party::MakeList(2, 2, 2)};
        const auto a0 = context.Parties[0].global_offset();
        const auto a1 = static_cast<oper_name_t>(context.Parties[0].global_offset()+1);
        const auto b0 = context.Parties[1].global_offset();
        const auto b1 = static_cast<oper_name_t>(context.Parties[1].global_offset()+1);
        LocalityOperatorSequenceGenerator osg{context, 1};

        ASSERT_EQ(osg.PartyCount(), 2);

        // Check Party A:
        EXPECT_EQ(osg.Party(0).party, &(context.Parties[0]));
        ASSERT_EQ(osg.Party(0).word_length(), 1);
        auto alice0 = osg.Party(0)[0];
        ASSERT_EQ(alice0.size(), 1);
        assert_is_id(alice0[0], context);

        auto alice1 = osg.Party(0)[1];
        ASSERT_EQ(alice1.size(), 2); // a0, a1
        EXPECT_EQ(alice1[0], OperatorSequence({a0}, context));
        EXPECT_EQ(alice1[1], OperatorSequence({a1}, context));

        // Check Party B:
        EXPECT_EQ(osg.Party(1).party, &(context.Parties[1]));
        ASSERT_EQ(osg.Party(1).word_length(), 1);
        auto bob0 = osg.Party(1)[0];
        ASSERT_EQ(bob0.size(), 1);
        assert_is_id(bob0[0], context);

        auto bob1 = osg.Party(1)[1];
        ASSERT_EQ(bob1.size(), 2); // b0, b1
        EXPECT_EQ(bob1[0], OperatorSequence({b0}, context));
        EXPECT_EQ(bob1[1], OperatorSequence({b1}, context));

        // Check whole sequence:
        ASSERT_EQ(osg.size(), 5); // id, a0, a1, b0, b1
        assert_sequence(osg.begin(), osg.end(), {
            OperatorSequence{context},
            OperatorSequence{{a0}, context},
            OperatorSequence{{a1}, context},
            OperatorSequence{{b0}, context},
            OperatorSequence{{b1}, context}
        });


    }
    
    TEST(Scenarios_Locality_LocalityOSG, CHSH_Length2) {
        LocalityContext context{Party::MakeList(2, 2, 2)};
        const auto a0 = context.Parties[0].global_offset();
        const auto a1 = static_cast<oper_name_t>(context.Parties[0].global_offset()+1);
        const auto b0 = context.Parties[1].global_offset();
        const auto b1 = static_cast<oper_name_t>(context.Parties[1].global_offset()+1);
        LocalityOperatorSequenceGenerator osg{context, 2};

        ASSERT_EQ(osg.PartyCount(), 2);

        // Check Party A:
        EXPECT_EQ(osg.Party(0).party, &(context.Parties[0]));
        ASSERT_EQ(osg.Party(0).word_length(), 2);
        auto alice0 = osg.Party(0)[0];
        ASSERT_EQ(alice0.size(), 1);
        assert_is_id(alice0[0], context);

        auto alice1 = osg.Party(0)[1];
        ASSERT_EQ(alice1.size(), 2); // a0, a1
        EXPECT_EQ(alice1[0], OperatorSequence({a0}, context));
        EXPECT_EQ(alice1[1], OperatorSequence({a1}, context));

        auto alice2 = osg.Party(0)[2];
        ASSERT_EQ(alice2.size(), 2); // a0a1, a1a0
        EXPECT_EQ(alice2[0], OperatorSequence({a0, a1}, context));
        EXPECT_EQ(alice2[1], OperatorSequence({a1, a0}, context));

        // Check Party B:
        EXPECT_EQ(osg.Party(1).party, &(context.Parties[1]));
        ASSERT_EQ(osg.Party(1).word_length(), 2);
        auto bob0 = osg.Party(1)[0];
        ASSERT_EQ(bob0.size(), 1);
        assert_is_id(bob0[0], context);

        auto bob1 = osg.Party(1)[1];
        ASSERT_EQ(bob1.size(), 2); // b0, b1
        EXPECT_EQ(bob1[0], OperatorSequence({b0}, context));
        EXPECT_EQ(bob1[1], OperatorSequence({b1}, context));

        auto bob2 = osg.Party(1)[2];
        ASSERT_EQ(bob2.size(), 2); // b0b1, b1b0
        EXPECT_EQ(bob2[0], OperatorSequence({b0, b1}, context));
        EXPECT_EQ(bob2[1], OperatorSequence({b1, b0}, context));

        // Check whole sequence:
        ASSERT_EQ(osg.size(), 13); // id, a0, a1, b0, b1, a0a1, a1a0, a0b0, a0b1, a1b0, a1b1, b0b1, b1b0
        assert_sequence(osg.begin(), osg.end(), {
                OperatorSequence{context},
                OperatorSequence{{a0}, context},
                OperatorSequence{{a1}, context},
                OperatorSequence{{b0}, context},
                OperatorSequence{{b1}, context},
                OperatorSequence{{a0, a1}, context},
                OperatorSequence{{a1, a0}, context},
                OperatorSequence{{a0, b0}, context},
                OperatorSequence{{a0, b1}, context},
                OperatorSequence{{a1, b0}, context},
                OperatorSequence{{a1, b1}, context},
                OperatorSequence{{b0, b1}, context},
                OperatorSequence{{b1, b0}, context}
        });
    }
}