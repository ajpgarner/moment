/**
 * alphabetic_namer_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "gtest/gtest.h"

#include "alphabetic_namer.h"

namespace NPATK::Tests {

    TEST(AlphabeticNamer, StrLen) {
        EXPECT_EQ(AlphabeticNamer::strlen(0), 1); // a
        EXPECT_EQ(AlphabeticNamer::strlen(25), 1); // z
        EXPECT_EQ(AlphabeticNamer::strlen(26), 2); // aa
        EXPECT_EQ(AlphabeticNamer::strlen(26 + (26*26) - 1), 2); // zz
        EXPECT_EQ(AlphabeticNamer::strlen(26 + (26*26)), 3); // aaa
        EXPECT_EQ(AlphabeticNamer::strlen(26 + (26*26) + (26*26*26) - 1), 3); // zzz
        EXPECT_EQ(AlphabeticNamer::strlen(26 + (26*26) + (26*26*26)), 4); // aaaa
    }

    TEST(AlphabeticNamer, LevelOffset) {
        EXPECT_EQ(AlphabeticNamer::level_offset(0), 0);
        EXPECT_EQ(AlphabeticNamer::level_offset(1), 26);
        EXPECT_EQ(AlphabeticNamer::level_offset(2), 26 + 26*26);
        EXPECT_EQ(AlphabeticNamer::level_offset(3), 26 + 26*26 + 26*26*26);
    }

    TEST(AlphabeticNamer, AtoZ_Lower) {
        AlphabeticNamer namer{false};
        EXPECT_EQ(namer(0), std::string("a"));
        EXPECT_EQ(namer(1), std::string("b"));
        EXPECT_EQ(namer(2), std::string("c"));
        EXPECT_EQ(namer(3), std::string("d"));
        EXPECT_EQ(namer(4), std::string("e"));
        EXPECT_EQ(namer(5), std::string("f"));
        EXPECT_EQ(namer(6), std::string("g"));
        EXPECT_EQ(namer(7), std::string("h"));
        EXPECT_EQ(namer(8), std::string("i"));
        EXPECT_EQ(namer(9), std::string("j"));
        EXPECT_EQ(namer(10), std::string("k"));
        EXPECT_EQ(namer(11), std::string("l"));
        EXPECT_EQ(namer(12), std::string("m"));
        EXPECT_EQ(namer(13), std::string("n"));
        EXPECT_EQ(namer(14), std::string("o"));
        EXPECT_EQ(namer(15), std::string("p"));
        EXPECT_EQ(namer(16), std::string("q"));
        EXPECT_EQ(namer(17), std::string("r"));
        EXPECT_EQ(namer(18), std::string("s"));
        EXPECT_EQ(namer(19), std::string("t"));
        EXPECT_EQ(namer(20), std::string("u"));
        EXPECT_EQ(namer(21), std::string("v"));
        EXPECT_EQ(namer(22), std::string("w"));
        EXPECT_EQ(namer(23), std::string("x"));
        EXPECT_EQ(namer(24), std::string("y"));
        EXPECT_EQ(namer(25), std::string("z"));
    }

    TEST(AlphabeticNamer, AtoZ_Upper) {
        AlphabeticNamer namer{true};

        EXPECT_EQ(namer(0), std::string("A"));
        EXPECT_EQ(namer(1), std::string("B"));
        EXPECT_EQ(namer(2), std::string("C"));
        EXPECT_EQ(namer(3), std::string("D"));
        EXPECT_EQ(namer(4), std::string("E"));
        EXPECT_EQ(namer(5), std::string("F"));
        EXPECT_EQ(namer(6), std::string("G"));
        EXPECT_EQ(namer(7), std::string("H"));
        EXPECT_EQ(namer(8), std::string("I"));
        EXPECT_EQ(namer(9), std::string("J"));
        EXPECT_EQ(namer(10), std::string("K"));
        EXPECT_EQ(namer(11), std::string("L"));
        EXPECT_EQ(namer(12), std::string("M"));
        EXPECT_EQ(namer(13), std::string("N"));
        EXPECT_EQ(namer(14), std::string("O"));
        EXPECT_EQ(namer(15), std::string("P"));
        EXPECT_EQ(namer(16), std::string("Q"));
        EXPECT_EQ(namer(17), std::string("R"));
        EXPECT_EQ(namer(18), std::string("S"));
        EXPECT_EQ(namer(19), std::string("T"));
        EXPECT_EQ(namer(20), std::string("U"));
        EXPECT_EQ(namer(21), std::string("V"));
        EXPECT_EQ(namer(22), std::string("W"));
        EXPECT_EQ(namer(23), std::string("X"));
        EXPECT_EQ(namer(24), std::string("Y"));
        EXPECT_EQ(namer(25), std::string("Z"));
    }

    TEST(AlphabeticNamer, AA_Lower) {
        AlphabeticNamer namer{false};
        EXPECT_EQ(namer(26), std::string("aa"));
        EXPECT_EQ(namer(51), std::string("az"));
        EXPECT_EQ(namer(52), std::string("ba"));
        EXPECT_EQ(namer(701), std::string("zz"));
        EXPECT_EQ(namer(702), std::string("aaa"));
    }

    TEST(AlphabeticNamer, AA_Upper) {
        AlphabeticNamer namer{true};
        EXPECT_EQ(namer(26), std::string("AA"));
        EXPECT_EQ(namer(51), std::string("AZ"));
        EXPECT_EQ(namer(52), std::string("BA"));
        EXPECT_EQ(namer(701), std::string("ZZ"));
        EXPECT_EQ(namer(702), std::string("AAA"));
    }
}