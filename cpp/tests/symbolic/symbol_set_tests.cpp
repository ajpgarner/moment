/**
 * symbol_set_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "symbolic/symbol_set.h"

namespace Moment::Tests {

    TEST(Symbolic_SymbolSet, Create_EmptySet) {
        std::vector<Symbol> empty_list{};

        auto ss = SymbolSet{empty_list};
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list has just one symbol.";

        auto symb_iter = ss.begin();
        ASSERT_NE(symb_iter, ss.end()) << "List should have one symbol to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);
        EXPECT_TRUE(symb_iter->second.is_zero());

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.end());

        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

    }


    TEST(Symbolic_SymbolSet, Create_ThreeUnlinkedSymbols) {
        std::vector<Symbol> three_symbols = {Symbol{0}, Symbol{1}, Symbol{2, false}};

        auto ss = SymbolSet{three_symbols};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        auto symb_iter = ss.begin();
        ASSERT_NE(symb_iter, ss.end()) << "List should have symbols to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);
        EXPECT_TRUE(symb_iter->second.is_zero());

        ++symb_iter;
        ASSERT_NE(symb_iter, ss.end());
        EXPECT_EQ(symb_iter->first, 1);
        EXPECT_EQ(symb_iter->second.id, 1);
        EXPECT_EQ(symb_iter->second.real_is_zero, false);
        EXPECT_EQ(symb_iter->second.im_is_zero, false);

        ++symb_iter;
        ASSERT_NE(symb_iter, ss.end());
        EXPECT_EQ(symb_iter->first, 2);
        EXPECT_EQ(symb_iter->second.id, 2);
        EXPECT_EQ(symb_iter->second.real_is_zero, false);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.end());
    }


    TEST(Symbolic_SymbolSet, Create_TwoUnlinkedWithMerge) {
        std::vector<Symbol> three_symbols = {Symbol{0}, Symbol{1}, Symbol{1, false}};

        auto ss = SymbolSet{three_symbols};
        ASSERT_EQ(ss.symbol_count(), 2) << "List should have two  symbols.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        auto symb_iter = ss.begin();
        ASSERT_NE(symb_iter, ss.end()) << "List should have symbols to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);
        EXPECT_TRUE(symb_iter->second.is_zero());

        ++symb_iter;
        ASSERT_NE(symb_iter, ss.end());
        EXPECT_EQ(symb_iter->first, 1);
        EXPECT_EQ(symb_iter->second.id, 1);
        EXPECT_EQ(symb_iter->second.real_is_zero, false);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.end());
    }


    TEST(Symbolic_SymbolSet, PackUnpack_EmptySet) {
        std::vector<Symbol> empty_list{};

        auto ss = SymbolSet{empty_list};
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list should have one symbol.";
        ASSERT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        ss.pack();
        ASSERT_EQ(ss.is_packed(), true) << "List should be packed after pack() command.";
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list should still have one symbol.";

        auto symb_iter = ss.begin();
        ASSERT_NE(symb_iter, ss.end()) << "List should have one symbol to iterate over!";
        EXPECT_EQ(symb_iter->first, 0);
        EXPECT_EQ(symb_iter->second.id, 0);
        EXPECT_EQ(symb_iter->second.real_is_zero, true);
        EXPECT_EQ(symb_iter->second.im_is_zero, true);

        ++symb_iter;
        ASSERT_EQ(symb_iter, ss.end());


        ss.unpack();
        ASSERT_EQ(ss.is_packed(), false) << "List should be unpacked after unpack() command.";
        ASSERT_EQ(ss.symbol_count(), 1) << "Empty list should still have one symbol.";

        auto up_iter = ss.begin();
        ASSERT_NE(up_iter, ss.end()) << "List should have one symbol to iterate over!";
        EXPECT_EQ(up_iter->first, 0);
        EXPECT_EQ(up_iter->second.id, 0);
        EXPECT_EQ(up_iter->second.real_is_zero, true);
        EXPECT_EQ(up_iter->second.im_is_zero, true);

        ++up_iter;
        ASSERT_EQ(up_iter, ss.end());
    }


    TEST(Symbolic_SymbolSet, PackUnpack_ClosedTriangle) {
        // 0 == 1, 0 == 2
        std::vector<Symbol> oet_list{Symbol{1}, Symbol{5}, Symbol{10}};
        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 4) << "List should have four symbols.";
        ASSERT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // Now pack..
        ss.pack();
        ASSERT_EQ(ss.is_packed(), true) << "List should be packed after pack() command.";
        ASSERT_EQ(ss.symbol_count(), 4) << "List should still have four symbols.";

        // First symbol
        auto sym_iter = ss.begin();
        ASSERT_NE(sym_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_iter->first, 0);
        EXPECT_EQ(sym_iter->second.id, 0);
        EXPECT_TRUE(sym_iter->second.is_zero());

        // Second symbol
        ++sym_iter;
        ASSERT_NE(sym_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_iter->first, 1);
        EXPECT_EQ(sym_iter->second.id, 1);

        // Third symbol
        ++sym_iter;
        ASSERT_NE(sym_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_iter->first, 2);
        EXPECT_EQ(sym_iter->second.id, 2);

        // Fourth symbol
        ++sym_iter;
        ASSERT_NE(sym_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_iter->first, 3);
        EXPECT_EQ(sym_iter->second.id, 3);

        // End
        ++sym_iter;
        ASSERT_EQ(sym_iter, ss.end()) << "Iteration should be over three symbols.";

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

        // First symbol
        auto sym_up_iter = ss.begin();
        ASSERT_NE(sym_up_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_up_iter->first, 0);
        EXPECT_EQ(sym_up_iter->second.id, 0);

        // Second symbol
        ++sym_up_iter;
        ASSERT_NE(sym_up_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(sym_up_iter->first, 1);
        EXPECT_EQ(sym_up_iter->second.id, 1);

        // Third symbol
        ++sym_up_iter;
        ASSERT_NE(sym_up_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_up_iter->first, 5);
        EXPECT_EQ(sym_up_iter->second.id, 5);;

        // Fourth symbol
        ++sym_up_iter;
        ASSERT_NE(sym_up_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(sym_up_iter->first, 10);
        EXPECT_EQ(sym_up_iter->second.id, 10);

        // End
        ++sym_up_iter;
        ASSERT_EQ(sym_up_iter, ss.end()) << "Iteration should be over three symbols.";


    }

}
