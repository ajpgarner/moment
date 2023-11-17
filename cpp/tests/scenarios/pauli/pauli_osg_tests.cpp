/**
 * pauli_osg_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/pauli/pauli_context.h"
#include "scenarios/pauli/pauli_osg.h"

namespace Moment::Tests {
    using namespace Moment::Pauli;

    class OSGTester {
    public:
        const OperatorSequenceGenerator& osg;
        const Context& context;
        std::vector<OperatorSequence>::const_iterator iter;
        const std::vector<OperatorSequence>::const_iterator iter_end;

        explicit OSGTester(const OperatorSequenceGenerator& osg)
            : osg{osg}, context{osg.context}, iter{osg.begin()}, iter_end{osg.end()} {

        }

        void check_and_advance(const OperatorSequence& reference) {
            ASSERT_NE(iter, iter_end);
            EXPECT_EQ(*iter, reference);
            ++iter;
        }

        void test_pauli_single(const size_t party) {
            const auto& pauli_context = dynamic_cast<const Pauli::PauliContext&>(context);
            check_and_advance(pauli_context.sigmaX(party));
            check_and_advance(pauli_context.sigmaY(party));
            check_and_advance(pauli_context.sigmaZ(party));
        }

        void test_pauli_pairs(const size_t party_A, const size_t party_B) {
            const auto& pauli_context = dynamic_cast<const Pauli::PauliContext&>(context);
            check_and_advance(pauli_context.sigmaX(party_A) * pauli_context.sigmaX(party_B));
            check_and_advance(pauli_context.sigmaX(party_A) * pauli_context.sigmaY(party_B));
            check_and_advance(pauli_context.sigmaX(party_A) * pauli_context.sigmaZ(party_B));
            check_and_advance(pauli_context.sigmaY(party_A) * pauli_context.sigmaX(party_B));
            check_and_advance(pauli_context.sigmaY(party_A) * pauli_context.sigmaY(party_B));
            check_and_advance(pauli_context.sigmaY(party_A) * pauli_context.sigmaZ(party_B));
            check_and_advance(pauli_context.sigmaZ(party_A) * pauli_context.sigmaX(party_B));
            check_and_advance(pauli_context.sigmaZ(party_A) * pauli_context.sigmaY(party_B));
            check_and_advance(pauli_context.sigmaZ(party_A) * pauli_context.sigmaZ(party_B));
        }

        void expected_finished() {
            EXPECT_EQ(iter, iter_end);
        }
    };

    TEST(Scenarios_Pauli_OSG, OneQubit_LevelZero) {
        PauliContext context{1};
        ASSERT_EQ(context.size(), 3);

        PauliSequenceGenerator psg{context, 0};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 1);
        auto psg_iter = psg.begin();
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, OperatorSequence(context));

        ++psg_iter;
        EXPECT_EQ(psg_iter, psg.end());
    }

    TEST(Scenarios_Pauli_OSG, OneQubit_LevelOne) {
        PauliContext context{1};
        ASSERT_EQ(context.size(), 3);

        PauliSequenceGenerator psg{context, 1};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 4);

        auto psg_iter = psg.begin();
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, OperatorSequence(context));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(0));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(0));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(0));

        ++psg_iter;
        EXPECT_EQ(psg_iter, psg.end());
    }

    TEST(Scenarios_Pauli_OSG, TwoQubits_LevelOne) {
        PauliContext context{2};
        ASSERT_EQ(context.size(), 6);

        PauliSequenceGenerator psg{context, 1};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 7);

        OSGTester tester{psg};
        tester.check_and_advance(OperatorSequence(context));
        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, TwoQubits_LevelTwo) {
        PauliContext context{2};
        ASSERT_EQ(context.size(), 6);

        PauliSequenceGenerator psg{context, 2};
        EXPECT_FALSE(psg.limited());
        EXPECT_EQ(psg.size(), 16);
        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);

        tester.test_pauli_pairs(0, 1);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, ThreeQubits_NearestNeighbours) {
        PauliContext context{3};
        ASSERT_EQ(context.size(), 9);

        PauliSequenceGenerator psg{context, 2, 1, false};
        
        EXPECT_TRUE(psg.limited());
        EXPECT_FALSE(psg.nearest_neighbour_index.wrapped);
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 28); // L0: 1, L1: 9; L2: 18

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(1, 2);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, ThreeQubits_NearestNeighboursWrapped) {
        PauliContext context{3};
        ASSERT_EQ(context.size(), 9);

        PauliSequenceGenerator psg{context, 2, 1, true};
        EXPECT_TRUE(psg.limited());
        EXPECT_TRUE(psg.nearest_neighbour_index.wrapped);
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 1);
        EXPECT_EQ(psg.size(), 37); // L0: 1, L1: 9; L2: 27

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(2, 0);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, FiveQubits_NextNearestNeighbours) {
        PauliContext context{5};
        ASSERT_EQ(context.size(), 15);

        PauliSequenceGenerator psg{context, 2, 2, false};
        EXPECT_TRUE(psg.limited());
        EXPECT_FALSE(psg.nearest_neighbour_index.wrapped);
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 2);
        EXPECT_EQ(psg.size(), 79); // L0: 1, L1: 15; L2: 63

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);
        tester.test_pauli_single(3);
        tester.test_pauli_single(4);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 2);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 3);
        tester.test_pauli_pairs(2, 3);
        tester.test_pauli_pairs(2, 4);
        tester.test_pauli_pairs(3, 4);

        tester.expected_finished();
    }

    TEST(Scenarios_Pauli_OSG, FiveQubits_NextNearestNeighboursWrapped) {
        PauliContext context{5};
        ASSERT_EQ(context.size(), 15);

        PauliSequenceGenerator psg{context, 2, 2, true};
        EXPECT_TRUE(psg.limited());
        EXPECT_TRUE(psg.nearest_neighbour_index.wrapped);
        EXPECT_EQ(psg.nearest_neighbour_index.neighbours, 2);
        EXPECT_EQ(psg.size(), 106); // L0: 1, L1: 15; L2: 90

        OSGTester tester{psg};

        tester.check_and_advance(OperatorSequence(context));

        tester.test_pauli_single(0);
        tester.test_pauli_single(1);
        tester.test_pauli_single(2);
        tester.test_pauli_single(3);
        tester.test_pauli_single(4);

        tester.test_pauli_pairs(0, 1);
        tester.test_pauli_pairs(0, 2);
        tester.test_pauli_pairs(1, 2);
        tester.test_pauli_pairs(1, 3);
        tester.test_pauli_pairs(2, 3);
        tester.test_pauli_pairs(2, 4);
        tester.test_pauli_pairs(3, 4);
        tester.test_pauli_pairs(3, 0);
        tester.test_pauli_pairs(4, 0);
        tester.test_pauli_pairs(4, 1);

        tester.expected_finished();
    }
}