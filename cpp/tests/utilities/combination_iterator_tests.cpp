/**
 * combination_iterator_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "utilities/combinations.h"

namespace NPATK::Tests {

    TEST(Utilities_Combinations, ComboIndex_Empty) {
        auto comboIter = CombinationIndexIterator{5, 0};
        EXPECT_EQ(comboIter.N, 5);
        EXPECT_EQ(comboIter.K, 0);
        EXPECT_FALSE(comboIter.done());
        EXPECT_EQ((*comboIter).size(), 0);
        EXPECT_EQ(comboIter->size(), 0);
        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }

    TEST(Utilities_Combinations, ComboIndex_N4K4) {
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

    TEST(Utilities_Combinations, ComboIndex_N3K1) {
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

    TEST(Utilities_Combinations, ComboIndex_N4K2) {
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



    namespace {
        void test_partition_vals(const PartitionIterator& comboIter,
                                 const std::vector<size_t>& expPrim,
                                 const std::vector<size_t>& expComp,
                                 const std::vector<bool>& expBits) {
            ASSERT_FALSE(comboIter.done());
            ASSERT_EQ(comboIter.primary().size(), expPrim.size());
            auto [prim, comp] = *comboIter;
            EXPECT_EQ(&prim, &comboIter.primary());
            EXPECT_EQ(&comp, &comboIter.complement());

            for (size_t pIndex = 0; pIndex < expPrim.size(); ++pIndex) {
                EXPECT_EQ(comboIter.primary(pIndex), expPrim[pIndex]) << "e:" << pIndex;
                EXPECT_EQ(comboIter.primary()[pIndex], expPrim[pIndex]) << "e:" << pIndex;
            }
            ASSERT_EQ(comboIter.complement().size(), expComp.size());
            for (size_t cIndex = 0; cIndex < expComp.size(); ++cIndex) {
                EXPECT_EQ(comboIter.complement(cIndex), expComp[cIndex]) << "e:" << cIndex;
                EXPECT_EQ(comboIter.complement()[cIndex], expComp[cIndex]) << "e:" << cIndex;
            }
            ASSERT_EQ(comboIter.bits().size(), expBits.size());
            for (size_t bIndex = 0; bIndex < expBits.size(); ++bIndex) {
                EXPECT_EQ(comboIter.bits(bIndex), expBits[bIndex]) << "e:" << bIndex;
                EXPECT_EQ(comboIter.bits()[bIndex], expBits[bIndex]) << "e:" << bIndex;
            }
        }
    }


    TEST(Utilities_Combinations, Partition_N5K0) {
        auto comboIter = PartitionIterator{5, 0};
        EXPECT_EQ(comboIter.N, 5);
        ASSERT_EQ(comboIter.K, 0);
        ASSERT_EQ(comboIter.NminusK, 5);

        test_partition_vals(comboIter, {}, {0, 1, 2, 3, 4}, {false, false, false, false, false});

        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }

    TEST(Utilities_Combinations, Partition_N5K5) {
        auto comboIter = PartitionIterator{5, 5};
        EXPECT_EQ(comboIter.N, 5);
        ASSERT_EQ(comboIter.K, 5);
        ASSERT_EQ(comboIter.NminusK, 0);

        test_partition_vals(comboIter, {0, 1, 2, 3, 4}, {}, {true, true, true, true, true});

        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }


    TEST(Utilities_Combinations, Partition_N3K1) {
        auto comboIter = PartitionIterator{3, 1};
        EXPECT_EQ(comboIter.N, 3);
        ASSERT_EQ(comboIter.K, 1);
        ASSERT_EQ(comboIter.NminusK, 2);

        test_partition_vals(comboIter, {0}, {1, 2}, {true, false, false});

        ++comboIter;
        test_partition_vals(comboIter, {1}, {0, 2}, {false, true, false});

        ++comboIter;
        test_partition_vals(comboIter, {2}, {0, 1}, {false, false, true});

        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }

    TEST(Utilities_Combinations, Partition_N4K2) {
        auto comboIter = PartitionIterator{4, 2};
        EXPECT_EQ(comboIter.N, 4);
        ASSERT_EQ(comboIter.K, 2);


        test_partition_vals(comboIter, {0, 1}, {2, 3}, {true, true, false, false});

        ++comboIter;
        test_partition_vals(comboIter, {0, 2}, {1, 3}, {true, false, true, false});

        ++comboIter;
        test_partition_vals(comboIter, {1, 2}, {0, 3}, {false, true, true, false});

        ++comboIter;
        test_partition_vals(comboIter, {0, 3}, {1, 2}, {true, false, false, true});

        ++comboIter;
        test_partition_vals(comboIter, {1, 3}, {0, 2}, {false, true, false, true});

        ++comboIter;
        test_partition_vals(comboIter, {2, 3}, {0, 1}, {false, false, true, true});

        ++comboIter;
        EXPECT_TRUE(comboIter.done());
    }

}