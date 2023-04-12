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

        bool anything = ams.ensure_osg_symbols(0);
        EXPECT_FALSE(anything);
    }

    TEST(Scenarios_WordList, EnsureOSG_Level2) {
        Algebraic::AlgebraicMatrixSystem ams{std::make_unique<Algebraic::AlgebraicContext>(2)};
        const auto& context = ams.AlgebraicContext();
        const auto& symbols = ams.Symbols();

        bool anything = ams.ensure_osg_symbols(2);
        EXPECT_TRUE(anything);
        EXPECT_EQ(symbols.size(), 7); // 0, e, a, b, aa, ab, bb

        const auto& wordlist = context.osg_list();
        EXPECT_EQ(wordlist.osg_index_to_symbol(0), std::make_pair(1LL, false)); // e -> 1
        EXPECT_EQ(wordlist.osg_index_to_symbol(1), std::make_pair(2LL, false)); // a -> 2
        EXPECT_EQ(wordlist.osg_index_to_symbol(2), std::make_pair(3LL, false)); // b -> 3
        EXPECT_EQ(wordlist.osg_index_to_symbol(3), std::make_pair(4LL, false)); // aa -> 4
        EXPECT_EQ(wordlist.osg_index_to_symbol(4), std::make_pair(5LL, false)); // ab -> 5
        EXPECT_EQ(wordlist.osg_index_to_symbol(5), std::make_pair(5LL, true)); // ba -> 5*
        EXPECT_EQ(wordlist.osg_index_to_symbol(6), std::make_pair(6LL, false)); // bb -> 6
        EXPECT_THROW(auto discard = wordlist.osg_index_to_symbol(7), std::range_error); // 7 not defined.

        bool nothing = ams.ensure_osg_symbols(2);
        EXPECT_FALSE(nothing);
        EXPECT_EQ(symbols.size(), 7);
    }

}