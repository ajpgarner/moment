/**
 * combination_iterator_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/combinations.h"

namespace NPATK::Tests {

    TEST(Combinations, ComboIndex_Empty) {
        auto comboIter = CombinationIndexIterator{5, 0};
        EXPECT_EQ(comboIter.N, 5);
        EXPECT_EQ(comboIter.K, 0);
        EXPECT_TRUE(comboIter.done());
    }

    TEST(Combinations, ComboIndex_N4K4) {
        auto comboIter = CombinationIndexIterator{4, 4};
        EXPECT_EQ(comboIter.N, 4);
        ASSERT_EQ(comboIter.K, 4);
        ASSERT_FALSE(comboIter.done());
        const auto& vals = *comboIter;
        const auto* ptrVals = comboIter.operator->();
        EXPECT_EQ(ptrVals, &vals);
        ASSERT_EQ(vals.size(), 4);
        EXPECT_EQ(vals[0], 0);
        EXPECT_EQ(vals[1], 1);
        EXPECT_EQ(vals[2], 2);
        EXPECT_EQ(vals[3], 3);
    }

    TEST(Combinations, ComboIndex_N3K1) {
        auto comboIter = CombinationIndexIterator{3, 1};
        EXPECT_EQ(comboIter.N, 3);
        ASSERT_EQ(comboIter.K, 1);
        ASSERT_FALSE(comboIter.done());

        ASSERT_EQ(comboIter->size(), 1);
        EXPECT_EQ((*comboIter)[0], 0);
        ++comboIter;

        ASSERT_EQ(comboIter->size(), 1);
        EXPECT_EQ((*comboIter)[0], 1);
        ++comboIter;

        ASSERT_EQ(comboIter->size(), 1);
        EXPECT_EQ((*comboIter)[0], 2);
        ++comboIter;

        EXPECT_TRUE(comboIter.done());
    }

    TEST(Combinations, ComboIndex_N4K2) {
        auto comboIter = CombinationIndexIterator{4, 2};
        EXPECT_EQ(comboIter.N, 4);
        ASSERT_EQ(comboIter.K, 2);

        ASSERT_FALSE(comboIter.done());
        ASSERT_EQ(comboIter->size(), 2);
        EXPECT_EQ((*comboIter)[0], 0);
        EXPECT_EQ((*comboIter)[1], 1);

        ++comboIter;
        ASSERT_EQ(comboIter->size(), 2);
        EXPECT_EQ((*comboIter)[0], 0);
        EXPECT_EQ((*comboIter)[1], 2);

        ++comboIter;
        ASSERT_EQ(comboIter->size(), 2);
        EXPECT_EQ((*comboIter)[0], 1);
        EXPECT_EQ((*comboIter)[1], 2);

        ++comboIter;
        ASSERT_EQ(comboIter->size(), 2);
        EXPECT_EQ((*comboIter)[0], 0);
        EXPECT_EQ((*comboIter)[1], 3);

        ++comboIter;
        ASSERT_EQ(comboIter->size(), 2);
        EXPECT_EQ((*comboIter)[0], 1);
        EXPECT_EQ((*comboIter)[1], 3);

        ++comboIter;
        ASSERT_EQ(comboIter->size(), 2);
        EXPECT_EQ((*comboIter)[0], 2);
        EXPECT_EQ((*comboIter)[1], 3);

        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }


    TEST(Combinations, Partition_N3K1) {
        auto comboIter = PartitionIterator{3, 1};
        EXPECT_EQ(comboIter.N, 3);
        ASSERT_EQ(comboIter.K, 1);
        ASSERT_EQ(comboIter.NminusK, 2);
        ASSERT_FALSE(comboIter.done());

        auto [prim, comp] = *comboIter;
        EXPECT_EQ(&prim, &comboIter.primary());
        EXPECT_EQ(&comp, &comboIter.complement());


        ASSERT_EQ(comboIter.primary().size(), 1);
        ASSERT_EQ(comboIter.complement().size(), 2);
        EXPECT_EQ(comboIter.primary(0), 0);
        EXPECT_EQ(comboIter.complement(0), 1);
        EXPECT_EQ(comboIter.complement(1), 2);

        ++comboIter;
        ASSERT_EQ(comboIter.primary().size(), 1);
        ASSERT_EQ(comboIter.complement().size(), 2);
        EXPECT_EQ(comboIter.primary(0), 1);
        EXPECT_EQ(comboIter.complement(0), 0);
        EXPECT_EQ(comboIter.complement(1), 2);

        ++comboIter;
        ASSERT_EQ(comboIter.primary().size(), 1);
        ASSERT_EQ(comboIter.complement().size(), 2);
        EXPECT_EQ(comboIter.primary(0), 2);
        EXPECT_EQ(comboIter.complement(0), 0);
        EXPECT_EQ(comboIter.complement(1), 1);

        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }

}