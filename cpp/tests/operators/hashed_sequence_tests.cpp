/**
 * hashed_sequence_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/hashed_sequence.h"
#include "operators/shortlex_hasher.h"

namespace NPATK::Tests {

    namespace {
        ptrdiff_t suffix_prefix(const HashedSequence& lhs, const HashedSequence& rhs) {
            return lhs.suffix_prefix_overlap(rhs);
        }

        ptrdiff_t suffix_prefix(const std::vector<oper_name_t>& lhs, const std::vector<oper_name_t>& rhs) {
            ShortlexHasher hasher{100};
            HashedSequence lhsHash{lhs, hasher};
            HashedSequence rhsHash{rhs, hasher};

            return lhsHash.suffix_prefix_overlap(rhsHash);
        }
    }

    TEST(HashedSequence, Match_ABinABAB) {
        std::vector<oper_name_t> sampleStr{3, 4, 3, 4};

        HashedSequence msr{{3, 4}, ShortlexHasher{5}};

        EXPECT_TRUE(msr.matches(sampleStr.begin(), sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 1, sampleStr.end()));
        EXPECT_TRUE(msr.matches(sampleStr.begin() + 2, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 3, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.end(), sampleStr.end()));

        auto matchA = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        EXPECT_EQ(matchA, sampleStr.begin());
        auto matchB = msr.matches_anywhere(matchA + 1, sampleStr.end());
        EXPECT_EQ(matchB, sampleStr.begin() + 2);
        auto matchC = msr.matches_anywhere(matchB + 1, sampleStr.end());
        EXPECT_EQ(matchC, sampleStr.end());
    }

    TEST(HashedSequence, Match_ABinBABA) {
        std::vector<oper_name_t> sampleStr{4, 3, 4, 3};

        HashedSequence msr{{3, 4}, ShortlexHasher{5}};

        EXPECT_FALSE(msr.matches(sampleStr.begin(), sampleStr.end()));
        EXPECT_TRUE(msr.matches(sampleStr.begin() + 1, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 2, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.begin() + 3, sampleStr.end()));
        EXPECT_FALSE(msr.matches(sampleStr.end(), sampleStr.end()));

        auto matchA = msr.matches_anywhere(sampleStr.begin(), sampleStr.end());
        EXPECT_EQ(matchA, sampleStr.begin() + 1);
        auto matchB = msr.matches_anywhere(matchA + 1, sampleStr.end());
        EXPECT_EQ(matchB, sampleStr.end());
    }

    TEST(HashedSequence, SuffixPrefix_None) {

        std::vector<oper_name_t> seqA{0, 1, 2};
        std::vector<oper_name_t> seqB{3, 4, 5};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(HashedSequence, SuffixPrefix_None_OneEmpty) {
        std::vector<oper_name_t> seqA{};
        std::vector<oper_name_t> seqB{0, 1, 2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(HashedSequence, SuffixPrefix_Complete) {
        std::vector<oper_name_t> seqA{0, 1, 2};
        std::vector<oper_name_t> seqB{0, 1, 2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 3);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 3);
        EXPECT_EQ(suffix_prefix(seqA, seqA), 3);
        EXPECT_EQ(suffix_prefix(seqB, seqB), 3);
    }

    TEST(HashedSequence, SuffixPrefix_OverlapOne) {
        std::vector<oper_name_t> seqA{0, 1, 2};
        std::vector<oper_name_t> seqB{2, 3, 4};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 1);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(HashedSequence, SuffixPrefix_OverlapOne_short) {
        std::vector<oper_name_t> seqA{0, 1, 2};
        std::vector<oper_name_t> seqB{2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 1);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }


    TEST(HashedSequence, SuffixPrefix_OverlapTwo) {
        std::vector<oper_name_t> seqA{0, 1, 2, 3};
        std::vector<oper_name_t> seqB{2, 3, 4};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 2);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(HashedSequence, SuffixPrefix_OverlapTwo_alt) {
        std::vector<oper_name_t> seqA{0, 1, 2, 3};
        std::vector<oper_name_t> seqB{2, 0, 1};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 2);
    }

    TEST(HashedSequence, Conjugate) {
        ShortlexHasher hasher{4};
        HashedSequence seqA{{0, 1, 2, 3}, hasher};
        HashedSequence seqB{{3, 2, 1, 0}, hasher};

        auto conjA = seqA.conjugate(hasher);
        EXPECT_EQ(conjA.hash, seqB.hash);
        ASSERT_EQ(conjA.size(), 4);
        EXPECT_EQ(conjA[0], 3);
        EXPECT_EQ(conjA[1], 2);
        EXPECT_EQ(conjA[2], 1);
        EXPECT_EQ(conjA[3], 0);
        EXPECT_EQ(conjA, seqB);

        auto conjB = seqB.conjugate(hasher);
        EXPECT_EQ(conjB.hash, seqA.hash);
        ASSERT_EQ(conjB.size(), 4);
        EXPECT_EQ(conjB[0], 0);
        EXPECT_EQ(conjB[1], 1);
        EXPECT_EQ(conjB[2], 2);
        EXPECT_EQ(conjB[3], 3);
        EXPECT_EQ(conjB, seqA);

    }
}
