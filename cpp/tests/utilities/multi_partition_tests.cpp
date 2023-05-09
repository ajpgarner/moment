/**
 * multi_partition_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "utilities/multi_partition.h"

namespace Moment::Tests {

    namespace {
        template<typename mpi_type>
        void assert_sequence(mpi_type& mpi,
                             std::initializer_list<std::initializer_list<int>> list_of_lists) {
            size_t grand_idx = 0;
            for (const auto& expected_seq : list_of_lists) {
                ASSERT_TRUE(mpi) << "index = " << grand_idx;
                const auto& actual = *mpi;
                ASSERT_EQ(actual.size(), expected_seq.size()) << "index = " << grand_idx;
                size_t idx = 0;
                for (const auto expected_elem : expected_seq) {
                    EXPECT_EQ(actual[idx], expected_elem) << "index = " << grand_idx << " element = " << idx;
                    ++idx;
                }
                ++grand_idx;
                ++mpi;
            }
            EXPECT_FALSE(mpi);
        }
    }

    TEST(Utilities_MultiPartition, N5P1) {
        MultipartitionIterator mpi(5, 1);
        ASSERT_EQ(mpi.Sum, 5);
        ASSERT_EQ(mpi.Parties, 1);
        EXPECT_TRUE(mpi);
        ASSERT_EQ((*mpi).size(), 1);
        EXPECT_EQ((*mpi)[0], 5);

        ++mpi;
        EXPECT_FALSE(mpi);

    }

    TEST(Utilities_MultiPartition, N3P2) {
        MultipartitionIterator mpi(3, 2);
        ASSERT_EQ(mpi.Sum, 3);
        ASSERT_EQ(mpi.Parties, 2);

        assert_sequence(mpi, {{0, 3}, {1, 2}, {2, 1}, {3, 0}});
    }

    TEST(Utilities_MultiPartition, N3P3) {
        MultipartitionIterator mpi(3, 3);
        ASSERT_EQ(mpi.Sum, 3);
        ASSERT_EQ(mpi.Parties, 3);

        assert_sequence(mpi, {{0, 0, 3}, {0, 1, 2}, {0, 2, 1}, {0, 3, 0},
                              {1, 0, 2}, {1, 1, 1}, {1, 2, 0},
                              {2, 0, 1}, {2, 1, 0},
                              {3 ,0 ,0}});
    }

    TEST(Utilities_MultiPartition, N2P4) {
        MultipartitionIterator mpi(2, 4);
        ASSERT_EQ(mpi.Sum, 2);
        ASSERT_EQ(mpi.Parties, 4);

        assert_sequence(mpi, {{0, 0, 0, 2}, {0, 0, 1, 1}, {0, 0, 2, 0},
                              {0, 1, 0, 1}, {0, 1, 1, 0}, {0, 2, 0, 0},
                              {1, 0, 0, 1}, {1, 0, 1, 0}, {1, 1, 0, 0},
                              {2, 0, 0, 0}});
    }

    TEST(Utilities_MultiPartition, N1P5) {
        MultipartitionIterator mpi(1, 5);
        ASSERT_EQ(mpi.Sum, 1);
        ASSERT_EQ(mpi.Parties, 5);

        assert_sequence(mpi, {{0, 0, 0, 0, 1},
                              {0, 0, 0, 1, 0},
                              {0, 0, 1, 0, 0},
                              {0, 1, 0, 0, 0},
                              {1, 0, 0, 0, 0}});
    }

    TEST(Utilities_MultiPartition, N3P4) {
        MultipartitionIterator mpi(3, 4);
        ASSERT_EQ(mpi.Sum, 3);
        ASSERT_EQ(mpi.Parties, 4);

        assert_sequence(mpi, {{0, 0, 0, 3}, {0, 0, 1, 2}, {0, 0, 2, 1}, {0, 0, 3, 0},
                              {0, 1, 0, 2}, {0, 1, 1, 1}, {0, 1, 2, 0},
                              {0, 2, 0, 1}, {0, 2, 1, 0}, {0, 3, 0, 0},
                              {1, 0, 0, 2}, {1, 0, 1, 1}, {1, 0, 2, 0},
                              {1, 1, 0, 1}, {1, 1, 1, 0}, {1, 2, 0, 0},
                              {2, 0, 0, 1}, {2, 0, 1, 0}, {2, 1, 0, 0},
                              {3, 0, 0, 0}});
    }

    TEST(Utilities_MultiPartition, Reversed_N5P1) {
        MultipartitionIterator<int, true> mpi(5, 1);
        ASSERT_EQ(mpi.Sum, 5);
        ASSERT_EQ(mpi.Parties, 1);
        EXPECT_TRUE(mpi);
        ASSERT_EQ((*mpi).size(), 1);
        EXPECT_EQ((*mpi)[0], 5);

        ++mpi;
        EXPECT_FALSE(mpi);

    }

    TEST(Utilities_MultiPartition, Reversed_N3P2) {
        MultipartitionIterator<int, true> mpi(3, 2);
        ASSERT_EQ(mpi.Sum, 3);
        ASSERT_EQ(mpi.Parties, 2);

        assert_sequence(mpi, {{3, 0}, {2, 1}, {1, 2}, {0, 3}});
    }

    TEST(Utilities_MultiPartition, Reversed_N3P3) {
        MultipartitionIterator<int, true> mpi(3, 3);
        ASSERT_EQ(mpi.Sum, 3);
        ASSERT_EQ(mpi.Parties, 3);

        assert_sequence(mpi, {{3 ,0 ,0},
                              {2, 1, 0}, {2, 0, 1},
                              {1, 2, 0}, {1, 1, 1}, {1, 0, 2},
                              {0, 3, 0}, {0, 2, 1}, {0, 1, 2}, {0, 0, 3}});
    }

    TEST(Utilities_MultiPartition, Reversed_N2P4) {
        MultipartitionIterator<int, true> mpi(2, 4);
        ASSERT_EQ(mpi.Sum, 2);
        ASSERT_EQ(mpi.Parties, 4);

        assert_sequence(mpi, {{2, 0, 0, 0},
                              {1, 1, 0, 0}, {1, 0, 1, 0}, {1, 0, 0, 1},
                              {0, 2, 0, 0}, {0, 1, 1, 0}, {0, 1, 0, 1},
                              {0, 0, 2, 0}, {0, 0, 1, 1}, {0, 0, 0, 2}});
    }

    TEST(Utilities_MultiPartition, Reversed_N1P5) {
        MultipartitionIterator<int, true> mpi(1, 5);
        ASSERT_EQ(mpi.Sum, 1);
        ASSERT_EQ(mpi.Parties, 5);

        assert_sequence(mpi, {{1, 0, 0, 0, 0},
                              {0, 1, 0, 0, 0},
                              {0, 0, 1, 0, 0},
                              {0, 0, 0, 1, 0},
                              {0, 0, 0, 0, 1}});
    }

    TEST(Utilities_MultiPartition, Reversed_N3P4) {
        MultipartitionIterator<int, true> mpi(3, 4);
        ASSERT_EQ(mpi.Sum, 3);
        ASSERT_EQ(mpi.Parties, 4);

        assert_sequence(mpi, {{3, 0, 0, 0},
                              {2, 1, 0, 0}, {2, 0, 1, 0}, {2, 0, 0, 1},
                              {1, 2, 0, 0}, {1, 1, 1, 0}, {1, 1, 0, 1},
                              {1, 0, 2, 0}, {1, 0, 1, 1}, {1, 0, 0, 2},
                              {0, 3, 0, 0}, {0, 2, 1, 0}, {0, 2, 0, 1},
                              {0, 1, 2, 0}, {0, 1, 1, 1}, {0, 1, 0, 2},
                              {0, 0, 3, 0}, {0, 0, 2, 1}, {0, 0, 1, 2}, {0, 0, 0, 3}});
    }
}