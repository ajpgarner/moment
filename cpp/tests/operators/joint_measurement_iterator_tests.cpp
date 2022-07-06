/**
 * multi_mmt_iterator_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "operators/context.h"
#include "operators/moment_matrix.h"
#include "operators/joint_measurement_iterator.h"

namespace NPATK::Tests {

    TEST(JointMeasurementIterator, BasicIteration) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 2, 2));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto &alice = contextPtr->Parties[0];
        const auto &bob = contextPtr->Parties[1];
        JointMeasurementIterator mmIter{*contextPtr, {&alice, &bob}};
        ASSERT_FALSE(mmIter.done());

        ASSERT_EQ(mmIter.indices().size(), 2);
        EXPECT_EQ(mmIter.indices()[0], 0);
        EXPECT_EQ(mmIter.indices()[1], 0);

        ASSERT_EQ(mmIter.global_indices().size(), 2);
        EXPECT_EQ(mmIter.global_indices()[0], 0);
        EXPECT_EQ(mmIter.global_indices()[1], 2);

        ++mmIter;
        ASSERT_FALSE(mmIter.done());
        ASSERT_EQ(mmIter.indices().size(), 2);
        EXPECT_EQ(mmIter.indices()[0], 0);
        EXPECT_EQ(mmIter.indices()[1], 1);

        ASSERT_EQ(mmIter.global_indices().size(), 2);
        EXPECT_EQ(mmIter.global_indices()[0], 0);
        EXPECT_EQ(mmIter.global_indices()[1], 3);

        ++mmIter;
        ASSERT_EQ(mmIter.indices().size(), 2);
        EXPECT_EQ(mmIter.indices()[0], 1);
        EXPECT_EQ(mmIter.indices()[1], 0);

        ASSERT_EQ(mmIter.global_indices().size(), 2);
        EXPECT_EQ(mmIter.global_indices()[0], 1);
        EXPECT_EQ(mmIter.global_indices()[1], 2);

        ++mmIter;
        ASSERT_FALSE(mmIter.done());
        ASSERT_EQ(mmIter.indices().size(), 2);
        EXPECT_EQ(mmIter.indices()[0], 1);
        EXPECT_EQ(mmIter.indices()[1], 1);

        ASSERT_EQ(mmIter.global_indices().size(), 2);
        EXPECT_EQ(mmIter.global_indices()[0], 1);
        EXPECT_EQ(mmIter.global_indices()[1], 3);

        ++mmIter;
        ASSERT_TRUE(mmIter.done());
    }

    namespace {
        void testOutcomeIter(const JointMeasurementIterator::OutcomeIndexIterator& iter,
                             const JointMeasurementIterator::OutcomeIndexIterator &iter_end,
                             const std::vector<size_t> &expectedIndices, const std::vector<bool> &expectedImpl) {
            const size_t vecSize = expectedIndices.size();

            ASSERT_NE(iter, iter_end);
            ASSERT_EQ((*iter).size(), vecSize);

            size_t impCount = 0;
            for (bool x : expectedImpl) {
                if (x) {
                    ++impCount;
                }
            }

            ASSERT_EQ(iter.implicit_count(), impCount);

            const auto &implVec = iter.implicit();
            for (size_t n = 0; n < vecSize; ++n) {
                EXPECT_EQ(iter[n], expectedIndices[n]) << n;
                EXPECT_EQ(implVec[n], expectedImpl[n]) << n;
            }
        }
    }

    TEST(JointMeasurementIterator, OutcomeIterator) {
        auto contextPtr = std::make_shared<Context>(Party::MakeList(2, 1, 2));
        ASSERT_EQ(contextPtr->Parties.size(), 2);
        const auto &alice = contextPtr->Parties[0];
        const auto &bob = contextPtr->Parties[1];
        JointMeasurementIterator mmIter{*contextPtr, {&alice, &bob}};
        ASSERT_FALSE(mmIter.done());
        ASSERT_EQ(mmIter.indices().size(), 2);
        EXPECT_EQ(mmIter.indices()[0], 0);
        EXPECT_EQ(mmIter.indices()[1], 0);
        ASSERT_EQ(mmIter.global_indices().size(), 2);
        EXPECT_EQ(mmIter.global_indices()[0], 0);
        EXPECT_EQ(mmIter.global_indices()[1], 1);

        auto outcomeIter = mmIter.begin_outcomes();
        const auto outcomeIterEnd = mmIter.end_outcomes();
        testOutcomeIter(outcomeIter, outcomeIterEnd, {0, 0}, {false, false});
        EXPECT_EQ(outcomeIter.explicit_outcome_index(), 0);

        ++outcomeIter;
        testOutcomeIter(outcomeIter, outcomeIterEnd, {0, 1}, {false, true});

        ++outcomeIter;
        testOutcomeIter(outcomeIter, outcomeIterEnd, {1, 0}, {true, false});

        ++outcomeIter;
        testOutcomeIter(outcomeIter, outcomeIterEnd, {1, 1}, {true, true});

        ++outcomeIter;
        EXPECT_EQ(outcomeIter, outcomeIterEnd);
    }
}