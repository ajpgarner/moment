/**
 * symbol_tests.cpp
 *
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"
#include "symbolic/monomial.h"

namespace Moment::Tests {

    TEST(Symbolic_SymbolExpression, Parse_One) {
        std::string one = "1";
        Monomial symbol{one};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_FALSE(symbol.negated());
        EXPECT_FALSE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), one);
    }


    TEST(Symbolic_SymbolExpression, Parse_Thirteen) {
        std::string thirteen = "13";
        Monomial symbol{thirteen};
        EXPECT_EQ(symbol.id, 13);
        EXPECT_FALSE(symbol.negated());
        EXPECT_FALSE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), thirteen);
    }

    TEST(Symbolic_SymbolExpression, Parse_MinusOne) {
        std::string minus_one = "-1";
        Monomial symbol{minus_one};
        EXPECT_EQ(symbol.id, 1);
        EXPECT_TRUE(symbol.negated());
        EXPECT_FALSE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), minus_one);
    }

    TEST(Symbolic_SymbolExpression, Parse_TwoStar) {
        std::string one_star = "2*";
        Monomial symbol{one_star};
        EXPECT_EQ(symbol.id, 2);
        EXPECT_FALSE(symbol.negated());
        EXPECT_TRUE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), one_star);
    }

    TEST(Symbolic_SymbolExpression, Parse_MinusTwoStar) {
        std::string minus_one_star = "-2*";
        Monomial symbol{minus_one_star};
        EXPECT_EQ(symbol.id, 2);
        EXPECT_TRUE(symbol.negated());
        EXPECT_TRUE(symbol.conjugated);
        EXPECT_EQ(symbol.as_string(), minus_one_star);
    }

    TEST(Symbolic_SymbolExpression, ComplexFactor) {
        Monomial symbolA{1, 2.0};
        Monomial symbolB{1, {2.0, 3.0}};
        EXPECT_FALSE(symbolA.complex_factor());
        EXPECT_TRUE(symbolB.complex_factor());
    }

    TEST(Symbolic_SymbolExpression, Equality) {
        Monomial symbolA{1, 2.0};
        Monomial symbolA_again{1, 2.0};
        Monomial symbolA_prime{1, 2.0, true};
        Monomial symbol2A{1, 4.0};
        Monomial symbolB{2, 2.0};

        EXPECT_TRUE(symbolA == symbolA_again);
        EXPECT_FALSE(symbolA == symbolA_prime);
        EXPECT_FALSE(symbolA == symbol2A);
        EXPECT_FALSE(symbolA == symbolB);
    }

    TEST(Symbolic_SymbolExpression, Equality_Zero) {
        Monomial zero{0};
        Monomial also_zero{0, 2.0};
        Monomial not_zero{1};

        EXPECT_TRUE(zero == also_zero);
        EXPECT_FALSE(zero == not_zero);
    }

    TEST(Symbolic_SymbolExpression, Inequality) {
        Monomial symbolA{1, 2.0};
        Monomial symbolA_again{1, 2.0};
        Monomial symbolA_prime{1, 2.0, true};
        Monomial symbol2A{1, 4.0};
        Monomial symbolB{2, 2.0};

        EXPECT_FALSE(symbolA != symbolA_again);
        EXPECT_TRUE(symbolA != symbolA_prime);
        EXPECT_TRUE(symbolA != symbol2A);
        EXPECT_TRUE(symbolA != symbolB);
    }

    TEST(Symbolic_SymbolExpression, Inequality_Zero) {
        Monomial zero{0};
        Monomial also_zero{0, 2.0};
        Monomial not_zero{1};

        EXPECT_FALSE(zero != also_zero);
        EXPECT_TRUE(zero != not_zero);
    }

    TEST(Symbolic_SymbolExpression, CopyConstruct) {
        const Monomial symbol{13, 2.0, true};
        const Monomial copied{symbol};
        EXPECT_EQ(symbol, copied);
        EXPECT_EQ(copied.id, 13);
        EXPECT_EQ(copied.factor, 2.0);
        EXPECT_TRUE(copied.conjugated);
    }

    TEST(Symbolic_SymbolExpression, BadStr_Empty) {
        std::string empty{};
        EXPECT_THROW(Monomial{empty}, Monomial::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_TooLong) {
        std::string longStr = std::string(Monomial::max_strlen + 1, '1');
        EXPECT_THROW(Monomial{longStr}, Monomial::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_NAN) {
        std::string badStr = "cheesecake";
        EXPECT_THROW(Monomial{badStr}, Monomial::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_DoubleMinus) {
        std::string badStr = "--100";
        EXPECT_THROW(Monomial{badStr}, Monomial::SymbolParseException);
    }

    TEST(Symbolic_SymbolExpression, BadStr_DoubleConj) {
        std::string badStr = "100**";
        EXPECT_THROW(Monomial{badStr}, Monomial::SymbolParseException);
    }
}