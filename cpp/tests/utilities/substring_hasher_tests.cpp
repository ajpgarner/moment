/**
 * substring_hasher_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/algebraic/algebraic_context.h"

#include "utilities/substring_hasher.h"

namespace Moment::Tests {
    using namespace Moment::Algebraic;

    TEST(Utilities_SubstringHasher, Empty) {
        AlgebraicContext ac{5};
        EXPECT_EQ(ac.size(), 5);
        const auto& hasher = ac.the_hasher();
        ASSERT_EQ(hasher.radix, 5);
        ASSERT_EQ(hasher.offset, 1);

        sequence_storage_t empty{};

        SubstringHashRange sshr{empty, hasher.radix};

        auto iter = sshr.begin();
        const auto iter_end = sshr.end();

        EXPECT_EQ(iter, iter_end);
    }


    TEST(Utilities_SubstringHasher, OneElement) {
        AlgebraicContext ac{5};
        EXPECT_EQ(ac.size(), 5);
        const auto& hasher = ac.the_hasher();
        ASSERT_EQ(hasher.radix, 5);
        ASSERT_EQ(hasher.offset, 1);

        sequence_storage_t str{4};

        SubstringHashRange sshr{str, hasher.radix};

        auto iter = sshr.begin();
        const auto iter_end = sshr.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(str));
        EXPECT_EQ(iter.index(), 0);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Utilities_SubstringHasher, TwoElements) {
        AlgebraicContext ac{6};
        EXPECT_EQ(ac.size(), 6);
        const auto& hasher = ac.the_hasher();
        ASSERT_EQ(hasher.radix, 6);
        ASSERT_EQ(hasher.offset, 1);

        sequence_storage_t str{4, 5};

        SubstringHashRange sshr{str, hasher.radix};

        auto iter = sshr.begin();
        const auto iter_end = sshr.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{5}));
        EXPECT_EQ(iter.index(), 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{4, 5}));
        EXPECT_EQ(iter.index(), 0);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{4}));
        EXPECT_EQ(iter.index(), 0);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Utilities_SubstringHasher, ThreeElems) {
        AlgebraicContext ac{3};
        EXPECT_EQ(ac.size(), 3);
        const auto& hasher = ac.the_hasher();
        ASSERT_EQ(hasher.radix, 3);
        ASSERT_EQ(hasher.offset, 1);

        sequence_storage_t str{0,1,2};

        SubstringHashRange sshr{str, hasher.radix};

        auto iter = sshr.begin();
        const auto iter_end = sshr.end();

        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{2}));
        EXPECT_EQ(iter.index(), 2);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{1, 2}));
        EXPECT_EQ(iter.index(), 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{0, 1, 2}));
        EXPECT_EQ(iter.index(), 0);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{1}));
        EXPECT_EQ(iter.index(), 1);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{0, 1}));
        EXPECT_EQ(iter.index(), 0);

        ++iter;
        ASSERT_NE(iter, iter_end);
        EXPECT_EQ(*iter, hasher.hash(sequence_storage_t{0}));
        EXPECT_EQ(iter.index(), 0);

        ++iter;
        EXPECT_EQ(iter, iter_end);
    }

    TEST(Utilities_SubstringHasher, ThreeElemsRange) {
        AlgebraicContext ac{5};
        EXPECT_EQ(ac.size(), 5);
        const auto& hasher = ac.the_hasher();
        ASSERT_EQ(hasher.radix, 5);
        ASSERT_EQ(hasher.offset, 1);

        sequence_storage_t str{0,4,2};

        const std::set<uint64_t> reference{
                hasher.hash(sequence_storage_t{0}),
                hasher.hash(sequence_storage_t{4}),
                hasher.hash(sequence_storage_t{2}),
                hasher.hash(sequence_storage_t{0, 4}),
                hasher.hash(sequence_storage_t{4, 2}),
                hasher.hash(sequence_storage_t{0, 4, 2})
        };
        ASSERT_EQ(reference.size(), 6);

        std::set<uint64_t> test;
        for (auto hash : SubstringHashRange{str, hasher.radix}) {
            ASSERT_TRUE(reference.contains(hash)) << "hash = " << hash;
            test.insert(hash);
        }
        ASSERT_EQ(test.size(), 6);
    }

}