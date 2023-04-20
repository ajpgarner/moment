/**
 * symbol_tests.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
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

    TEST(Symbolic_SymbolExpression, Equality) {
        SymbolExpression symbolA{1, 2.0};
        SymbolExpression symbolA_again{1, 2.0};
        SymbolExpression symbolA_prime{1, 2.0, true};
        SymbolExpression symbol2A{1, 4.0};
        SymbolExpression symbolB{2, 2.0};

        EXPECT_TRUE(symbolA == symbolA_again);
        EXPECT_FALSE(symbolA == symbolA_prime);
        EXPECT_FALSE(symbolA == symbol2A);
        EXPECT_FALSE(symbolA == symbolB);
    }

    TEST(Symbolic_SymbolExpression, Equality_Zero) {
        SymbolExpression zero{0};
        SymbolExpression also_zero{0, 2.0};
        SymbolExpression not_zero{1};

        EXPECT_TRUE(zero == also_zero);
        EXPECT_FALSE(zero == not_zero);
    }

    TEST(Symbolic_SymbolExpression, Inequality) {
        SymbolExpression symbolA{1, 2.0};
        SymbolExpression symbolA_again{1, 2.0};
        SymbolExpression symbolA_prime{1, 2.0, true};
        SymbolExpression symbol2A{1, 4.0};
        SymbolExpression symbolB{2, 2.0};

        EXPECT_FALSE(symbolA != symbolA_again);
        EXPECT_TRUE(symbolA != symbolA_prime);
        EXPECT_TRUE(symbolA != symbol2A);
        EXPECT_TRUE(symbolA != symbolB);
    }

    TEST(Symbolic_SymbolExpression, Inequality_Zero) {
        SymbolExpression zero{0};
        SymbolExpression also_zero{0, 2.0};
        SymbolExpression not_zero{1};

        EXPECT_FALSE(zero != also_zero);
        EXPECT_TRUE(zero != not_zero);
    }

    TEST(Symbolic_SymbolExpression, CopyConstruct) {
        const SymbolExpression symbol{13, 2.0, true};
        const SymbolExpression copied{symbol};
        EXPECT_EQ(symbol, copied);
        EXPECT_EQ(copied.id, 13);
        EXPECT_EQ(copied.factor, 2.0);
        EXPECT_TRUE(copied.conjugated);
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