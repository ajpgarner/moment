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

    TEST(Scenarios_Pauli_OSG, OneQubit_LevelZero) {
        PauliContext context{1};
        ASSERT_EQ(context.size(), 3);

        PauliSequenceGenerator psg{context, 0};
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
        EXPECT_EQ(psg.size(), 7);

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
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(1));

        ++psg_iter;
        EXPECT_EQ(psg_iter, psg.end());
    }

    TEST(Scenarios_Pauli_OSG, TwoQubits_LevelTwo) {
        PauliContext context{2};
        ASSERT_EQ(context.size(), 6);

        PauliSequenceGenerator psg{context, 2};
        EXPECT_EQ(psg.size(), 16);

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
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(0) * context.sigmaX(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(0) * context.sigmaY(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaX(0) * context.sigmaZ(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(0) * context.sigmaX(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(0) * context.sigmaY(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaY(0) * context.sigmaZ(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(0) * context.sigmaX(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(0) * context.sigmaY(1));

        ++psg_iter;
        ASSERT_NE(psg_iter, psg.end());
        EXPECT_EQ(*psg_iter, context.sigmaZ(0) * context.sigmaZ(1));

        ++psg_iter;
        EXPECT_EQ(psg_iter, psg.end());
    }
}