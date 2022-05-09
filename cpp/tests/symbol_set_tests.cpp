#include "gtest/gtest.h"

#include "symbol_set.h"

namespace NPATK::Tests {

    TEST(SymbolSet, Create_EmptySet) {
        std::vector<NPATK::SymbolPair> empty_list{};

        auto ss = SymbolSet{empty_list};
        ASSERT_EQ(ss.symbol_count(), 0) << "Empty list should have no symbols.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should have no links.";
        ASSERT_EQ(ss.begin(), ss.end()) << "Empty list should give us nothing to iterate over";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        ASSERT_EQ(ss.unpacked_names().size(), 0) << "Name set should be empty.";
    }

    TEST(SymbolSet, Create_OneLinkSet) {
        // 0 == 1
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(Symbol{0}, Symbol{1});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 2) << "List should have two symbols.";
        ASSERT_EQ(ss.link_count(), 1) << "List should have one link.";
        ASSERT_NE(ss.begin(), ss.end()) << "List should be iterable.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // Check symbol names
        EXPECT_EQ(ss.unpacked_names().size(), 2) << "Name set should have two symbols.";
        EXPECT_TRUE(ss.unpacked_names().contains(0));
        EXPECT_TRUE(ss.unpacked_names().contains(1));

        // First link
        auto ss_iter = ss.begin();
        ASSERT_NE(ss_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.end()) << "Iteration should be only over one link.";
    }

    TEST(SymbolSet, Create_OpenTriangle) {
        // 0 == 1, 0 == 2
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(Symbol{0}, Symbol{1});
        oet_list.emplace_back(Symbol{0}, Symbol{2});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        ASSERT_EQ(ss.link_count(), 2) << "List should have two links.";
        ASSERT_NE(ss.begin(), ss.end()) << "List should be iterable.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // Check symbol names
        EXPECT_EQ(ss.unpacked_names().size(), 3) << "Name set should have three symbols.";
        EXPECT_TRUE(ss.unpacked_names().contains(0));
        EXPECT_TRUE(ss.unpacked_names().contains(1));
        EXPECT_TRUE(ss.unpacked_names().contains(2));

        // First link
        auto ss_iter = ss.begin();
        ASSERT_NE(ss_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Second link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.end()) << "Iteration should be over two links.";
    }

    TEST(SymbolSet, Create_ClosedTriangle) {
        // 0 == 1, 0 == 2
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(Symbol{0}, Symbol{1});
        oet_list.emplace_back(Symbol{0}, Symbol{2});
        oet_list.emplace_back(Symbol{1}, Symbol{2});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        ASSERT_EQ(ss.link_count(), 3) << "List should have three links.";
        ASSERT_NE(ss.begin(), ss.end()) << "List should be iterable.";
        EXPECT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // Check symbol names
        EXPECT_EQ(ss.unpacked_names().size(), 3) << "Name set should have three symbols.";
        EXPECT_TRUE(ss.unpacked_names().contains(0));
        EXPECT_TRUE(ss.unpacked_names().contains(1));
        EXPECT_TRUE(ss.unpacked_names().contains(2));

        // First link
        auto ss_iter = ss.begin();
        ASSERT_NE(ss_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Second link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Third link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 1);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.end()) << "Iteration should be over three links.";
    }


    TEST(SymbolSet, PackUnpack_EmptySet) {
        std::vector<NPATK::SymbolPair> empty_list{};

        auto ss = SymbolSet{empty_list};
        ASSERT_EQ(ss.symbol_count(), 0) << "Empty list should have no symbols.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should have no links.";
        ASSERT_EQ(ss.begin(), ss.end()) << "Empty list should give us nothing to iterate over";
        ASSERT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        ss.pack();
        ASSERT_EQ(ss.is_packed(), true) << "List should be packed after pack() command.";
        ASSERT_EQ(ss.symbol_count(), 0) << "Empty list should still have no symbols.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should still have no links.";
        ASSERT_EQ(ss.begin(), ss.end()) << "Empty list should still give us nothing to iterate over";

        ss.unpack();
        ASSERT_EQ(ss.is_packed(), false) << "List should be unpacked after unpack() command.";
        ASSERT_EQ(ss.symbol_count(), 0) << "Empty list should still have no symbols.";
        ASSERT_EQ(ss.link_count(), 0) << "Empty list should still have no links.";
        ASSERT_EQ(ss.begin(), ss.end()) << "Empty list should still give us nothing to iterate over";
    }


    TEST(SymbolSet, PackUnpack_ClosedTriangle) {
        // 0 == 1, 0 == 2
        std::vector<NPATK::SymbolPair> oet_list{};
        oet_list.emplace_back(Symbol{1}, Symbol{5});
        oet_list.emplace_back(Symbol{1}, Symbol{10});
        oet_list.emplace_back(Symbol{5}, Symbol{10});

        auto ss = SymbolSet{oet_list};
        ASSERT_EQ(ss.symbol_count(), 3) << "List should have three symbols.";
        ASSERT_EQ(ss.link_count(), 3) << "List should have three links.";
        ASSERT_NE(ss.begin(), ss.end()) << "List should be iterable.";
        ASSERT_EQ(ss.is_packed(), false) << "Newly-created list should not be packed.";

        // Check symbol names
        EXPECT_EQ(ss.unpacked_names().size(), 3) << "Name set should have three symbols.";
        EXPECT_TRUE(ss.unpacked_names().contains(1));
        EXPECT_TRUE(ss.unpacked_names().contains(5));
        EXPECT_TRUE(ss.unpacked_names().contains(10));

        // Now pack..
        ss.pack();
        ASSERT_EQ(ss.is_packed(), true) << "List should be packed after pack() command.";
        ASSERT_EQ(ss.symbol_count(), 3) << "List should still have 3 symbols.";
        ASSERT_EQ(ss.link_count(), 3) << "List should still have 3 links.";
        ASSERT_NE(ss.begin(), ss.end()) << "List should still be iterable.";

        // First link
        auto ss_iter = ss.begin();
        ASSERT_NE(ss_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 1);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Second link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 0);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // Third link
        ++ss_iter;
        ASSERT_NE(ss_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_iter->first.first, 1);
        EXPECT_EQ(ss_iter->first.second, 2);
        EXPECT_EQ(ss_iter->second, EqualityType::equal);

        // End
        ++ss_iter;
        ASSERT_EQ(ss_iter, ss.end()) << "Iteration should be over three links.";

        // Now, unpack again
        ss.unpack();
        ASSERT_EQ(ss.is_packed(), false) << "List should be unpacked after unpack() command.";

        // First link
        auto ss_up_iter = ss.begin();
        ASSERT_NE(ss_up_iter, ss.end()) << "Iterator must point to something.";
        EXPECT_EQ(ss_up_iter->first.first, 1);
        EXPECT_EQ(ss_up_iter->first.second, 5);
        EXPECT_EQ(ss_up_iter->second, EqualityType::equal);

        // Second link
        ++ss_up_iter;
        ASSERT_NE(ss_up_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_up_iter->first.first, 1);
        EXPECT_EQ(ss_up_iter->first.second, 10);
        EXPECT_EQ(ss_up_iter->second, EqualityType::equal);

        // Third link
        ++ss_up_iter;
        ASSERT_NE(ss_up_iter, ss.end()) << "Iterator should not yet be at end.";
        EXPECT_EQ(ss_up_iter->first.first, 5);
        EXPECT_EQ(ss_up_iter->first.second, 10);
        EXPECT_EQ(ss_up_iter->second, EqualityType::equal);
        
    }

}
