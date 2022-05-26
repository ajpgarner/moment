/**
 * symbol_set_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "symbol_set.h"

namespace NPATK::Tests {

    TEST(SymbolSet, Create_EmptySet) {
        std::vector<NPATK::SymbolPair> empty_list{};

        auto ss = SymbolSet{empty_list};
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list has just one symbol.";

        auto symb_iter = ss.Symbols.begin();
        ASSERT_NE(symb_iter, ss.Symbols.end()) << "List should have one symbol to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);
        EXPECT_TRUE(symb_iter->second.is_zero());

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.Symbols.end());

        ASSERT_EQ(ss.link_count(), 0) << "Empty list should have no links.";
        ASSERT_EQ(ss.Links.begin(), ss.Links.end()) << "Empty list should give us nothing to iterate over";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

    }


    TEST(SymbolSet, Create_ThreeUnlinkedSymbols) {
        std::vector<NPATK::SymbolPair> empty_list{};
        std::vector<NPATK::Symbol> three_symbols = {Symbol{0}, Symbol{1}, Symbol{2, false}};

        auto ss = SymbolSet{three_symbols, empty_list};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        ASSERT_EQ(ss.link_count(), 0) << "List should have no links.";
        ASSERT_EQ(ss.Links.begin(), ss.Links.end()) << "Empty list should give us no links to iterate over";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        auto symb_iter = ss.Symbols.begin();
        ASSERT_NE(symb_iter, ss.Symbols.end()) << "List should have symbols to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);
        EXPECT_TRUE(symb_iter->second.is_zero());

        ++symb_iter;
        ASSERT_NE(symb_iter, ss.Symbols.end());
        EXPECT_EQ(symb_iter->first, 1);
        EXPECT_EQ(symb_iter->second.id, 1);
        EXPECT_EQ(symb_iter->second.real_is_zero, false);
        EXPECT_EQ(symb_iter->second.im_is_zero, false);

        ++symb_iter;
        ASSERT_NE(symb_iter, ss.Symbols.end());
        EXPECT_EQ(symb_iter->first, 2);
        EXPECT_EQ(symb_iter->second.id, 2);
        EXPECT_EQ(symb_iter->second.real_is_zero, false);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.Symbols.end());
    }


    TEST(SymbolSet, Create_TwoUnlinkedWithMerge) {
        std::vector<NPATK::SymbolPair> empty_list{};
        std::vector<NPATK::Symbol> three_symbols = {Symbol{0}, Symbol{1}, Symbol{1, false}};

        auto ss = SymbolSet{three_symbols, empty_list};
        ASSERT_EQ(ss.symbol_count(), 2) << "List should have two  symbols.";
        ASSERT_EQ(ss.link_count(), 0) << "List should have no links.";
        ASSERT_EQ(ss.Links.begin(), ss.Links.end()) << "Empty list should give us no links to iterate over";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        auto symb_iter = ss.Symbols.begin();
        ASSERT_NE(symb_iter, ss.Symbols.end()) << "List should have symbols to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);
        EXPECT_TRUE(symb_iter->second.is_zero());

        ++symb_iter;
        ASSERT_NE(symb_iter, ss.Symbols.end());
        EXPECT_EQ(symb_iter->first, 1);
        EXPECT_EQ(symb_iter->second.id, 1);
        EXPECT_EQ(symb_iter->second.real_is_zero, false);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.Symbols.end());
    }

    TEST(SymbolSet, Create_OneLinkSet) {
        // 0 == 1
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(SymbolExpression{0}, SymbolExpression{1});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 2) << "List should have two symbols.";
        ASSERT_EQ(ss.link_count(), 1) << "List should have one link.";
        ASSERT_NE(ss.Links.begin(), ss.Links.end()) << "List should be iterable.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // First link
        auto ss_iter = ss.Links.begin();
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.Links.end()) << "Iteration should be only over one link.";
    }

    TEST(SymbolSet, Create_OpenTriangle) {
        // 0 == 1, 0 == 2
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(SymbolExpression{0}, SymbolExpression{1});
        oet_list.emplace_back(SymbolExpression{0}, SymbolExpression{2});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        ASSERT_EQ(ss.link_count(), 2) << "List should have two links.";
        ASSERT_NE(ss.Links.begin(), ss.Links.end()) << "List should be iterable.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // First link
        auto ss_iter = ss.Links.begin();
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Second link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.Links.end()) << "Iteration should be over two links.";
    }

    TEST(SymbolSet, Create_ClosedTriangle) {
        // 0 == 1, 0 == 2
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(SymbolExpression{0}, SymbolExpression{1});
        oet_list.emplace_back(SymbolExpression{0}, SymbolExpression{2});
        oet_list.emplace_back(SymbolExpression{1}, SymbolExpression{2});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        ASSERT_EQ(ss.link_count(), 3) << "List should have three links.";
        ASSERT_NE(ss.Links.begin(), ss.Links.end()) << "List should be iterable.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // First link
        auto ss_iter = ss.Links.begin();
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Second link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Third link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 1);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.Links.end()) << "Iteration should be over three links.";
    }


    TEST(SymbolSet, PackUnpack_EmptySet) {
        std::vector<NPATK::SymbolPair> empty_list{};

        auto ss = SymbolSet{empty_list};
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list should have one symbol.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should have no links.";
        ASSERT_EQ(ss.Links.begin(), ss.Links.end()) << "Empty list should give us nothing to iterate over";
        ASSERT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        ss.pack();
        ASSERT_EQ(ss.is_packed(), true) << "List should be packed after pack() command.";
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list should still have one symbol.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should still have no links.";


        auto symb_iter = ss.Symbols.begin();
        ASSERT_NE(symb_iter, ss.Symbols.end()) << "List should have one symbol to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.Symbols.end());


        ss.unpack();
        ASSERT_EQ(ss.is_packed(), false) << "List should be unpacked after unpack() command.";
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list should still have one symbol.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should still have no links.";

        auto up_iter = ss.Symbols.begin();
        ASSERT_NE(up_iter, ss.Symbols.end()) << "List should have one symbol to iterate over!";
        EXPECT_EQ(up_iter->first, 0);
        EXPECT_EQ(up_iter->second.id, 0);
        EXPECT_EQ(up_iter->second.real_is_zero, true);
        EXPECT_EQ(up_iter->second.im_is_zero, true);

        ++up_iter;
        ASSERT_EQ(up_iter, ss.Symbols.end());
    }


    TEST(SymbolSet, PackUnpack_ClosedTriangle) {
        // 0 == 1, 0 == 2
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(SymbolExpression{1}, SymbolExpression{5});
        oet_list.emplace_back(SymbolExpression{1}, SymbolExpression{10});
        oet_list.emplace_back(SymbolExpression{5}, SymbolExpression{10});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 4) << "List should have four symbols.";
        ASSERT_EQ(ss.link_count(), 3) << "List should have three links.";
        ASSERT_NE(ss.Links.begin(), ss.Links.end()) << "List should be iterable.";
        ASSERT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // Now pack..
        ss.pack();
        ASSERT_EQ(ss.is_packed(), true) << "List should be packed after pack() command.";
        ASSERT_EQ(ss.symbol_count(), 4) << "List should still have four symbols.";
        ASSERT_EQ(ss.link_count(), 3) << "List should still have 3 links.";
        ASSERT_NE(ss.Links.begin(), ss.Links.end()) << "List should still be iterable.";

        // First link
        auto ss_iter = ss.Links.begin();
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 1);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Second link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 1);
        EXPECT_EQ(ss_iter->first.second, 3);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Third link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 2);
        EXPECT_EQ(ss_iter->first.second, 3);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.Links.end()) << "Iteration should be over three links.";

        // First symbol
        auto sym_iter = ss.Symbols.begin();
        ASSERT_NE(sym_iter, ss.Symbols.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_iter->first, 0);
        EXPECT_EQ(sym_iter->second.id, 0);
        EXPECT_TRUE(sym_iter->second.is_zero());

        // Second symbol
        ++sym_iter;
        ASSERT_NE(sym_iter, ss.Symbols.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_iter->first, 1);
        EXPECT_EQ(sym_iter->second.id, 1);

        // Third symbol
        ++sym_iter;
        ASSERT_NE(sym_iter, ss.Symbols.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_iter->first, 2);
        EXPECT_EQ(sym_iter->second.id, 2);

        // Fourth symbol
        ++sym_iter;
        ASSERT_NE(sym_iter, ss.Symbols.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_iter->first, 3);
        EXPECT_EQ(sym_iter->second.id, 3);

        // End
        ++sym_iter;
        ASSERT_EQ(sym_iter, ss.Symbols.end()) << "Iteration should be over three symbols.";

        auto [fk1, upk1] = ss.packed_key(1);
        ASSERT_TRUE(fk1);
        EXPECT_EQ(upk1, 1);

        auto [fk2, upk2] = ss.packed_key(5);
        ASSERT_TRUE(fk2);
        EXPECT_EQ(upk2, 2);

        auto [fk3, upk3] = ss.packed_key(10);
        ASSERT_TRUE(fk3);
        EXPECT_EQ(upk3, 3);

        auto [fk4, upk4] = ss.packed_key(20);
        ASSERT_FALSE(fk4);


        auto [gk0, pk0] = ss.unpacked_key(0);
        ASSERT_TRUE(gk0);
        EXPECT_EQ(pk0, 0);

        auto [gk1, pk1] = ss.unpacked_key(1);
        ASSERT_TRUE(gk1);
        EXPECT_EQ(pk1, 1);

        auto [gk2, pk2] = ss.unpacked_key(2);
        ASSERT_TRUE(gk2);
        EXPECT_EQ(pk2, 5);

        auto [gk3, pk3] = ss.unpacked_key(3);
        ASSERT_TRUE(gk3);
        EXPECT_EQ(pk3, 10);

        auto [gk4, pk4] = ss.unpacked_key(-1);
        ASSERT_FALSE(gk4);

        auto [gk5, pk5] = ss.unpacked_key(4);
        ASSERT_FALSE(gk5);


        // Now, unpack again
        ss.unpack();
        ASSERT_EQ(ss.is_packed(), false) << "List should be unpacked after unpack() command.";

        // First link
        auto ss_up_iter = ss.Links.begin();
        ASSERT_NE(ss_up_iter, ss.Links.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_up_iter->first.first, 1);
        EXPECT_EQ(ss_up_iter->first.second, 5);
        EXPECT_EQ(ss_up_iter->second, EqualityType::equal);

        // Second link
        ++ss_up_iter;
        ASSERT_NE(ss_up_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_up_iter->first.first, 1);
        EXPECT_EQ(ss_up_iter->first.second, 10);
        EXPECT_EQ(ss_up_iter->second, EqualityType::equal);

        // Third link
        ++ss_up_iter;
        ASSERT_NE(ss_up_iter, ss.Links.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_up_iter->first.first, 5);
        EXPECT_EQ(ss_up_iter->first.second, 10);
        EXPECT_EQ(ss_up_iter->second, EqualityType::equal);

        // First symbol
        auto sym_up_iter = ss.Symbols.begin();
        ASSERT_NE(sym_up_iter, ss.Symbols.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_up_iter->first, 0);
        EXPECT_EQ(sym_up_iter->second.id, 0);

        // Second symbol
        ++sym_up_iter;
        ASSERT_NE(sym_up_iter, ss.Symbols.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_up_iter->first, 1);
        EXPECT_EQ(sym_up_iter->second.id, 1);

        // Third symbol
        ++sym_up_iter;
        ASSERT_NE(sym_up_iter, ss.Symbols.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_up_iter->first, 5);
        EXPECT_EQ(sym_up_iter->second.id, 5);;

        // Fourth symbol
        ++sym_up_iter;
        ASSERT_NE(sym_up_iter, ss.Symbols.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_up_iter->first, 10);
        EXPECT_EQ(sym_up_iter->second.id, 10);

        // End
        ++sym_up_iter;
        ASSERT_EQ(sym_up_iter, ss.Symbols.end()) << "Iteration should be over three symbols.";


    }

}
