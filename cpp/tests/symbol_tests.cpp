/**
 * symbol_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"
#include "symbol.h"

namespace NPATK::Tests {

    TEST(SymbolExpression, Parse_One) {
        std::string one = "1";
        SymbolExpression symbol{one};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_FALSE(symbol.negated);
        EXPECT_FALSE(symbol.conjugated);
    }


    TEST(SymbolExpression, Parse_Thirteen) {
        std::string thirteen = "13";
        SymbolExpression symbol{thirteen};
        EXPECT_EQ(symbol.id, 13);
        EXPECT_FALSE(symbol.negated);
        EXPECT_FALSE(symbol.conjugated);
    }

    TEST(SymbolExpression, Parse_MinusOne) {
        std::string thirteen = "-1";
        SymbolExpression symbol{thirteen};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_TRUE(symbol.negated);
        EXPECT_FALSE(symbol.conjugated);
    }

    TEST(SymbolExpression, Parse_OneStar) {
        std::string thirteen = "1*";
        SymbolExpression symbol{thirteen};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_FALSE(symbol.negated);
        EXPECT_TRUE(symbol.conjugated);
    }

    TEST(SymbolExpression, Parse_MinusOneStar) {
        std::string thirteen = "-1*";
        SymbolExpression symbol{thirteen};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_TRUE(symbol.negated);
        EXPECT_TRUE(symbol.conjugated);
    }

    TEST(SymbolExpression, BadStr_Empty) {
        std::string empty{};
        EXPECT_THROW(SymbolExpression{empty}, SymbolExpression::SymbolParseException);
    }

    TEST(SymbolExpression, BadStr_TooLong) {
        std::string longStr = std::string(SymbolExpression::max_strlen+1, '1');
        EXPECT_THROW(SymbolExpression{longStr}, SymbolExpression::SymbolParseException);
    }

    TEST(SymbolExpression, BadStr_NAN) {
        std::string badStr = "cheesecake";
        EXPECT_THROW(SymbolExpression{badStr}, SymbolExpression::SymbolParseException);
    }

    TEST(SymbolExpression, BadStr_DoubleMinus) {
        std::string badStr = "--100";
        EXPECT_THROW(SymbolExpression{badStr}, SymbolExpression::SymbolParseException);
    }

    TEST(SymbolExpression, BadStr_DoubleConj) {
        std::string badStr = "100**";
        EXPECT_THROW(SymbolExpression{badStr}, SymbolExpression::SymbolParseException);
    }
}