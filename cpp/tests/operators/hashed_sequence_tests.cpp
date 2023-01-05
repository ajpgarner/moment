/**
 * hashed_sequence_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "hashed_sequence.h"
#include "utilities/shortlex_hasher.h"

namespace Moment::Tests {

    namespace {
        ptrdiff_t suffix_prefix(const HashedSequence& lhs, const HashedSequence& rhs) {
            return lhs.suffix_prefix_overlap(rhs);
        }

        ptrdiff_t suffix_prefix(const sequence_storage_t & lhs, const sequence_storage_t& rhs) {
            ShortlexHasher hasher{100};
            HashedSequence lhsHash{lhs, hasher};
            HashedSequence rhsHash{rhs, hasher};

            return lhsHash.suffix_prefix_overlap(rhsHash);
        }
    }

    TEST(Operators_HashedSequence, Match_ABinABAB) {
        sequence_storage_t  sampleStr{3, 4, 3, 4};

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

    TEST(Operators_HashedSequence, Match_ABinBABA) {
        sequence_storage_t  sampleStr{4, 3, 4, 3};

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

    TEST(Operators_HashedSequence, SuffixPrefix_None) {

        sequence_storage_t  seqA{0, 1, 2};
        sequence_storage_t  seqB{3, 4, 5};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(Operators_HashedSequence, SuffixPrefix_None_OneEmpty) {
        sequence_storage_t  seqA{};
        sequence_storage_t  seqB{0, 1, 2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(Operators_HashedSequence, SuffixPrefix_Complete) {
        sequence_storage_t  seqA{0, 1, 2};
        sequence_storage_t  seqB{0, 1, 2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 3);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 3);
        EXPECT_EQ(suffix_prefix(seqA, seqA), 3);
        EXPECT_EQ(suffix_prefix(seqB, seqB), 3);
    }

    TEST(Operators_HashedSequence, SuffixPrefix_OverlapOne) {
        sequence_storage_t  seqA{0, 1, 2};
        sequence_storage_t  seqB{2, 3, 4};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 1);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(Operators_HashedSequence, SuffixPrefix_OverlapOne_short) {
        sequence_storage_t  seqA{0, 1, 2};
        sequence_storage_t  seqB{2};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 1);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }


    TEST(Operators_HashedSequence, SuffixPrefix_OverlapTwo) {
        sequence_storage_t  seqA{0, 1, 2, 3};
        sequence_storage_t  seqB{2, 3, 4};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 2);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 0);
    }

    TEST(Operators_HashedSequence, SuffixPrefix_OverlapTwo_alt) {
        sequence_storage_t  seqA{0, 1, 2, 3};
        sequence_storage_t  seqB{2, 0, 1};

        EXPECT_EQ(suffix_prefix(seqA, seqB), 0);
        EXPECT_EQ(suffix_prefix(seqB, seqA), 2);
    }

    TEST(Operators_HashedSequence, Conjugate) {
        ShortlexHasher hasher{4};
        HashedSequence seqA{{0, 1, 2, 3}, hasher};
        HashedSequence seqB{{3, 2, 1, 0}, hasher};

        auto conjA = seqA.conjugate(hasher);
        EXPECT_EQ(conjA.hash(), seqB.hash());
        ASSERT_EQ(conjA.size(), 4);
        EXPECT_EQ(conjA[0], 3);
        EXPECT_EQ(conjA[1], 2);
        EXPECT_EQ(conjA[2], 1);
        EXPECT_EQ(conjA[3], 0);
        EXPECT_EQ(conjA, seqB);

        auto conjB = seqB.conjugate(hasher);
        EXPECT_EQ(conjB.hash(), seqA.hash());
        ASSERT_EQ(conjB.size(), 4);
        EXPECT_EQ(conjB[0], 0);
        EXPECT_EQ(conjB[1], 1);
        EXPECT_EQ(conjB[2], 2);
        EXPECT_EQ(conjB[3], 3);
        EXPECT_EQ(conjB, seqA);

    }

    TEST(Operators_HashedSequence, Conjugate_Zero) {
        ShortlexHasher hasher{4};
        HashedSequence seqA{true};

        auto conjA = seqA.conjugate(hasher);
        EXPECT_EQ(conjA.hash(), seqA.hash());
        ASSERT_EQ(conjA.size(), 0);
        EXPECT_TRUE(conjA.zero());
    }

    TEST(Operators_HashedSequence, Conjugate_Id) {
        ShortlexHasher hasher{4};
        HashedSequence seqA{{}, hasher};

        auto conjA = seqA.conjugate(hasher);
        EXPECT_EQ(conjA.hash(), seqA.hash());
        ASSERT_EQ(conjA.size(), 0);
        EXPECT_FALSE(conjA.zero());
    }
}
