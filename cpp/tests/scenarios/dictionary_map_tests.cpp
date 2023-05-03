/**
 * word_list_tests.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "gtest/gtest.h"

#include "scenarios/word_list.h"
#include "scenarios/algebraic/algebraic_context.h"
#include "scenarios/algebraic/algebraic_matrix_system.h"

#include "symbolic/symbol_table.h"

#include <memory>

namespace Moment::Tests {
    TEST(Scenarios_WordList, EnsureOSG_Empty) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();

        bool anything = ams.generate_dictionary(0);
        EXPECT_FALSE(anything);
        EXPECT_EQ(symbols.OSGIndex.max_length(), 0);
    }

    TEST(Scenarios_WordList, EnsureOSG_Level2) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();

        EXPECT_EQ(symbols.OSGIndex.max_length(), 0);
        bool anything = ams.generate_dictionary(2);
        EXPECT_TRUE(anything);
        EXPECT_EQ(symbols.size(), 7); // 0, e, a, b, aa, ab, bb
        EXPECT_EQ(symbols.OSGIndex.max_length(), 2);

        const auto& wordlist = symbols.OSGIndex;
        EXPECT_EQ(wordlist(0).first, 1LL); // e -> 1
        EXPECT_EQ(wordlist(0).second, false); // e -> 1
        EXPECT_EQ(wordlist(1).first, 2LL); // a -> 2
        EXPECT_EQ(wordlist(1).second, false); // a -> 2
        EXPECT_EQ(wordlist(2).first, 3LL); // b -> 3
        EXPECT_EQ(wordlist(2).second,  false); // b -> 3
        EXPECT_EQ(wordlist(3).first, 4LL); // aa -> 4
        EXPECT_EQ(wordlist(3).second, false); // aa -> 4
        EXPECT_EQ(wordlist(4).first, 5LL); // ab -> 5
        EXPECT_EQ(wordlist(4).second, false); // ab -> 5
        EXPECT_EQ(wordlist(5).first, 5LL); // ba -> 5*
        EXPECT_EQ(wordlist(5).second, true); // ba -> 5*
        EXPECT_EQ(wordlist(6).first, 6LL); // bb -> 6
        EXPECT_EQ(wordlist(6).second, false); // bb -> 6
        EXPECT_THROW(auto discard = wordlist(7), std::range_error); // 7 not defined.

        bool nothing = ams.generate_dictionary(2);
        EXPECT_FALSE(nothing);
        EXPECT_EQ(symbols.size(), 7);
    }

}