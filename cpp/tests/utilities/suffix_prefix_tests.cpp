/**
 * suffix_prefix_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"
#include "utilities/suffix_prefix.h"

namespace NPATK::Tests {

    TEST(SuffixPrefix, None) {
        
        std::vector<size_t> seqA{0, 1, 2};
        std::vector<size_t> seqB{3, 4, 5};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(SuffixPrefix, None_OneEmpty) {
        std::vector<size_t> seqA{};
        std::vector<size_t> seqB{0, 1, 2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(SuffixPrefix, Complete) {
        std::vector<size_t> seqA{0, 1, 2};
        std::vector<size_t> seqB{0, 1, 2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 3);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 3);
        EXPECT_EQ(suffix_prefix(seqA, seqA), 3);
        EXPECT_EQ(suffix_prefix(seqB, seqB), 3);
    }

    TEST(SuffixPrefix, OverlapOne) {
        std::vector<size_t> seqA{0, 1, 2};
        std::vector<size_t> seqB{2, 3, 4};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 1);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(SuffixPrefix, OverlapOne_short) {
        std::vector<size_t> seqA{0, 1, 2};
        std::vector<size_t> seqB{2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 1);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }


    TEST(SuffixPrefix, OverlapTwo) {
        std::vector<size_t> seqA{0, 1, 2, 3};
        std::vector<size_t> seqB{2, 3, 4};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 2);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(SuffixPrefix, OverlapTwo_alt) {
        std::vector<size_t> seqA{0, 1, 2, 3};
        std::vector<size_t> seqB{2, 0, 1};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 2);
    }
}