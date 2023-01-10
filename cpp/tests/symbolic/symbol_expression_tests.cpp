/**
 * symbol_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"
#include "symbolic/symbol_expression.h"

namespace Moment::Tests {

    TEST(Symbolic_SymbolExpression, Parse_One) {
        std::string one = "1";
        SymbolExpression symbol{one};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_FALSE(symbol.negated());
        EXPECT_FALSE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), one);
    }


    TEST(Symbolic_SymbolExpression, Parse_Thirteen) {
        std::string thirteen = "13";
        SymbolExpression symbol{thirteen};
        EXPECT_EQ(symbol.id, 13);
        EXPECT_FALSE(symbol.negated());
        EXPECT_FALSE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), thirteen);
    }

    TEST(Symbolic_SymbolExpression, Parse_MinusOne) {
        std::string minus_one = "-1";
        SymbolExpression symbol{minus_one};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_TRUE(symbol.negated());
        EXPECT_FALSE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), minus_one);
    }

    TEST(Symbolic_SymbolExpression, Parse_OneStar) {
        std::string one_star = "1*";
        SymbolExpression symbol{one_star};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_FALSE(symbol.negated());
        EXPECT_TRUE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), one_star);
    }

    TEST(Symbolic_SymbolExpression, Parse_MinusOneStar) {
        std::string minus_one_star = "-1*";
        SymbolExpression symbol{minus_one_star};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_TRUE(symbol.negated());
        EXPECT_TRUE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), minus_one_star);
    }

    TEST(Symbolic_SymbolExpression, BadStr_Empty) {
        std::string empty{};
        EXPECT_THROW(SymbolExpression{empty}, SymbolExpression::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_TooLong) {
        std::string longStr = std::string(SymbolExpression::max_strlen+1, '1');
        EXPECT_THROW(SymbolExpression{longStr}, SymbolExpression::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_NAN) {
        std::string badStr = "cheesecake";
        EXPECT_THROW(SymbolExpression{badStr}, SymbolExpression::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_DoubleMinus) {
        std::string badStr = "--100";
        EXPECT_THROW(SymbolExpression{badStr}, SymbolExpression::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_DoubleConj) {
        std::string badStr = "100**";
        EXPECT_THROW(SymbolExpression{badStr}, SymbolExpression::SymbolParseException);
    }
}