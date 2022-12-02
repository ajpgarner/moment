/**
 * shortlex_hasher_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "operators/shortlex_hasher.h"


namespace NPATK::Tests {

    TEST(Operators_ShortlexHasher, Unary) {
        ShortlexHasher hasher{1};
        ASSERT_EQ(hasher.radix, 1);
        auto e = hasher({});
        auto a = hasher({0});
        auto aa = hasher({0, 0});
        auto aaa = hasher({0, 0, 0});

        EXPECT_LT(e, a);
        EXPECT_LT(a, aa);
        EXPECT_LT(aa, aaa);
    }

    TEST(Operators_ShortlexHasher, Binary) {
        ShortlexHasher hasher{2};
        ASSERT_EQ(hasher.radix, 2);
        auto e = hasher({});
        auto a = hasher({0});
        auto b = hasher({1});
        auto aa = hasher({0, 0});
        auto ab = hasher({0, 1});
        auto ba = hasher({1, 0});
        auto bb = hasher({1, 1});
        auto aaa = hasher({0, 0, 0});

        EXPECT_LT(e, a);
        EXPECT_LT(a, b);
        EXPECT_LT(b, aa);
        EXPECT_LT(aa, ab);
        EXPECT_LT(ab, ba);
        EXPECT_LT(ba, bb);
        EXPECT_LT(bb, aaa);
    }

    TEST(Operators_ShortlexHasher, Trinary) {
        ShortlexHasher hasher{3};
        ASSERT_EQ(hasher.radix, 3);
        auto e = hasher({});
        auto a = hasher({0});
        auto b = hasher({1});
        auto c = hasher({2});
        auto aa = hasher({0, 0});
        auto ab = hasher({0, 1});
        auto ac = hasher({0, 2});
        auto ba = hasher({1, 0});
        auto bb = hasher({1, 1});
        auto bc = hasher({1, 2});
        auto ca = hasher({2, 0});
        auto cb = hasher({2, 1});
        auto cc = hasher({2, 2});
        auto aaa = hasher({0, 0, 0});

        EXPECT_LT(e, a);
        EXPECT_LT(a, b);
        EXPECT_LT(b, c);
        EXPECT_LT(c, aa);
        EXPECT_LT(aa, ab);
        EXPECT_LT(ab, ac);
        EXPECT_LT(ac, ba);
        EXPECT_LT(ba, bb);
        EXPECT_LT(bb, bc);
        EXPECT_LT(bc, ca);
        EXPECT_LT(ca, cb);
        EXPECT_LT(cb, cc);
        EXPECT_LT(cc, aaa);
    }

}