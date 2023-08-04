/**
 * index_tree_tests.cpp
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "gtest/gtest.h"

#include "utilities/index_tree.h"

namespace Moment::Tests {

    TEST(Utilities_IndexTree, Empty) {
        IndexTree<int, size_t> tree{};
        EXPECT_TRUE(tree.leaf());

        auto f = tree.find(std::vector<int>{});
        EXPECT_FALSE(f.has_value());

        // Root
        const auto* basePtr = tree.find_node(std::vector<int>{});
        EXPECT_EQ(basePtr, &tree);
    }

    TEST(Utilities_IndexTree, Singleton) {
        IndexTree<int, size_t> tree{};

        tree.add(std::vector{12}, 52);
        EXPECT_FALSE(tree.leaf());

        auto val = tree.find(std::vector{12});
        ASSERT_TRUE(val.has_value());
        EXPECT_EQ(val.value(), 52);

        // Root
        const auto* basePtr = tree.find_node(std::vector<int>{});
        EXPECT_EQ(basePtr, &tree);

        // Child
        const auto* childPtr = tree.find_node(std::vector<int>{12});
        ASSERT_NE(childPtr, nullptr);
        ASSERT_TRUE(childPtr->index().has_value());
        EXPECT_EQ(childPtr->index().value(), 52);

    }

    TEST(Utilities_IndexTree, Pair_Ordered) {
        IndexTree<int, size_t> tree{};

        tree.add(std::vector{3}, 10);
        tree.add(std::vector{12}, 20);
        EXPECT_FALSE(tree.leaf());

        auto valA = tree.find(std::vector{3});
        ASSERT_TRUE(valA.has_value());
        EXPECT_EQ(valA.value(), 10);

        auto valB = tree.find(std::vector{12});
        ASSERT_TRUE(valB.has_value());
        EXPECT_EQ(valB.value(), 20);

        // Root
        const auto* basePtr = tree.find_node(std::vector<int>{});
        EXPECT_EQ(basePtr, &tree);

        // Child
        const auto* childAPtr = tree.find_node(std::vector<int>{3});
        ASSERT_NE(childAPtr, nullptr);
        ASSERT_TRUE(childAPtr->index().has_value());
        EXPECT_EQ(childAPtr->index().value(), 10);
        
        // Child
        const auto* childBPtr = tree.find_node(std::vector<int>{12});
        ASSERT_NE(childBPtr, nullptr);
        ASSERT_TRUE(childBPtr->index().has_value());
        EXPECT_EQ(childBPtr->index().value(), 20);

    }

    TEST(Utilities_IndexTree, Pair_Unordered) {
        IndexTree<int, size_t> tree{};

        tree.add(std::vector{12}, 20);
        tree.add(std::vector{3}, 10);
        EXPECT_FALSE(tree.leaf());

        auto missingVal = tree.find(std::vector{4});
        EXPECT_FALSE(missingVal.has_value());

        auto valA = tree.find(std::vector{3});
        ASSERT_TRUE(valA.has_value());
        EXPECT_EQ(valA.value(), 10);

        auto valB = tree.find(std::vector{12});
        ASSERT_TRUE(valB.has_value());
        EXPECT_EQ(valB.value(), 20);

        // Root
        const auto* basePtr = tree.find_node(std::vector<int>{});
        EXPECT_EQ(basePtr, &tree);

        // Child
        const auto* childAPtr = tree.find_node(std::vector<int>{3});
        ASSERT_NE(childAPtr, nullptr);
        ASSERT_TRUE(childAPtr->index().has_value());
        EXPECT_EQ(childAPtr->index().value(), 10);

        // Child
        const auto* childBPtr = tree.find_node(std::vector<int>{12});
        ASSERT_NE(childBPtr, nullptr);
        ASSERT_TRUE(childBPtr->index().has_value());
        EXPECT_EQ(childBPtr->index().value(), 20);

    }

    TEST(Utilities_IndexTree, OneString) {
        IndexTree<int, size_t> tree{};

        tree.add(std::vector{1, 2, 3}, 13);
        EXPECT_FALSE(tree.leaf());

        auto missingVal = tree.find(std::vector{3, 2, 1});
        EXPECT_FALSE(missingVal.has_value());

        auto val_direct = tree.find(std::vector{1, 2, 3});
        ASSERT_TRUE(val_direct.has_value());
        EXPECT_EQ(val_direct.value(), 13);

        EXPECT_FALSE(tree.find(std::vector<int>{}).has_value());
        EXPECT_FALSE(tree.find(std::vector{1}).has_value());
        EXPECT_FALSE(tree.find(std::vector{1, 2}).has_value());


        // Root
        const auto* basePtr = tree.find_node(std::vector<int>{});
        EXPECT_EQ(basePtr, &tree);

        // Child
        const auto* childAPtr = tree.find_node(1);
        ASSERT_NE(childAPtr, nullptr);
        EXPECT_FALSE(childAPtr->index().has_value());

        // Grandchild
        const auto* grandChildPtr = childAPtr->find_node(2);
        ASSERT_NE(grandChildPtr, nullptr);
        EXPECT_FALSE(grandChildPtr->index().has_value());

        // Great-grandchild
        const auto* greatGrandChildPtr = grandChildPtr->find_node(3);
        ASSERT_NE(greatGrandChildPtr, nullptr);
        ASSERT_TRUE(greatGrandChildPtr->index().has_value());
        EXPECT_EQ(greatGrandChildPtr->index().value(), 13);

        // Alternative search
        const auto* altGGCPtr = childAPtr->find_node(std::vector{2, 3});
        EXPECT_EQ(altGGCPtr, greatGrandChildPtr);

    }


    TEST(Utilities_IndexTree, Tree) {
        IndexTree<int, size_t> tree{};

        tree.add(std::vector{1, 2, 3}, 13);
        tree.add(std::vector{1}, 10);
        tree.add(std::vector{1, 2, 4}, 17);
        EXPECT_FALSE(tree.leaf());


        auto val_13 = tree.find(std::vector{1, 2, 3});
        ASSERT_TRUE(val_13.has_value());
        EXPECT_EQ(val_13.value(), 13);

        auto val_10 = tree.find(std::vector{1});
        ASSERT_TRUE(val_10.has_value());
        EXPECT_EQ(val_10.value(), 10);

        auto val_17 = tree.find(std::vector{1, 2, 4});
        ASSERT_TRUE(val_17.has_value());
        EXPECT_EQ(val_17.value(), 17);

        EXPECT_FALSE(tree.find(std::vector<int>{}).has_value());
        EXPECT_FALSE(tree.find(std::vector{1, 2}).has_value());
        EXPECT_FALSE(tree.find(std::vector{5}).has_value());
    }

    TEST(Utilities_IndexTree, AddIfNew) {
        IndexTree<int, size_t> tree{};
        auto [e0, i0] = tree.add_if_new(std::vector{1, 2, 3}, 13);
        EXPECT_EQ(e0, 13);
        EXPECT_TRUE(i0);

        auto [e1, i1] = tree.add_if_new(std::vector{1}, 10);
        EXPECT_EQ(e1, 10);
        EXPECT_TRUE(i1);

        auto [e2, i2] = tree.add_if_new(std::vector{1, 2, 4}, 17);
        EXPECT_EQ(e2, 17);
        EXPECT_TRUE(i2);

        auto [e3, i3] = tree.add_if_new(std::vector{1, 2, 3}, 99);
        EXPECT_EQ(e3, 13);
        EXPECT_FALSE(i3);

        auto val_13 = tree.find(std::vector{1, 2, 3});
        ASSERT_TRUE(val_13.has_value());
        EXPECT_EQ(val_13.value(), 13);

        auto val_10 = tree.find(std::vector{1});
        ASSERT_TRUE(val_10.has_value());
        EXPECT_EQ(val_10.value(), 10);

        auto val_17 = tree.find(std::vector{1, 2, 4});
        ASSERT_TRUE(val_17.has_value());
        EXPECT_EQ(val_17.value(), 17);

        EXPECT_FALSE(tree.find(std::vector<int>{}).has_value());
        EXPECT_FALSE(tree.find(std::vector{1, 2}).has_value());
        EXPECT_FALSE(tree.find(std::vector{5}).has_value());
    }

    TEST(Utilities_IndexTree, FindWithHint) {
        IndexTree<int, size_t> tree{};
        tree.add(std::vector{3}, 10);
        tree.add(std::vector{12}, 20);
        tree.add(std::vector{12, 5}, 30);
        ASSERT_EQ(tree.find(std::vector{12, 5}).value(), 30);

        const auto* twelve_node = tree.find_node(std::vector{12});
        ASSERT_NE(twelve_node, nullptr);

        std::vector search{12, 8};
        auto [hint_ptr, remainder] = tree.find_node_or_return_hint(search);
        EXPECT_EQ(hint_ptr, twelve_node);
        ASSERT_EQ(remainder.size(), 1);
        EXPECT_EQ(remainder[0], 8);

        tree.add(std::vector{12, 8}, 40);
        auto maybe_val = hint_ptr->find(remainder);
        ASSERT_TRUE(maybe_val.has_value());
        EXPECT_EQ(maybe_val.value(), 40);


    }

    TEST(Utilities_IndexTree, Iterator) {
        IndexTree<int, size_t> tree{};
        tree.add(std::vector{1}, 10);
        tree.add(std::vector{1, 2, 3}, 13);
        tree.add(std::vector{1, 2, 4}, 17);
        tree.add(std::vector{1, 3}, 20);

        auto tree_iter = tree.begin();
        const auto tree_iter_end = tree.end();

        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 0);
        EXPECT_EQ(tree_iter.lookup_index(), std::vector<int>());
        EXPECT_FALSE(tree_iter->index().has_value());

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 1);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 10);

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 2);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 2}));
        EXPECT_FALSE(tree_iter->index().has_value());

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 3);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 2, 3}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 13);

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 3);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 2, 4}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 17);

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 2);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 3}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 20);

        ++tree_iter;
        EXPECT_EQ(tree_iter, tree_iter_end);

    }

    TEST(Utilities_IndexTree, ConstIterator) {
        IndexTree<int, size_t> tree{};
        tree.add(std::vector{1}, 10);
        tree.add(std::vector{1, 2, 3}, 13);
        tree.add(std::vector{1, 2, 4}, 17);
        tree.add(std::vector{1, 3}, 20);

        auto tree_iter = tree.cbegin();
        const auto tree_iter_end = tree.cend();

        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 0);
        EXPECT_EQ(tree_iter.lookup_index(), std::vector<int>());
        EXPECT_FALSE(tree_iter->index().has_value());

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 1);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 10);

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 2);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 2}));
        EXPECT_FALSE(tree_iter->index().has_value());

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 3);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 2, 3}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 13);

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 3);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 2, 4}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 17);

        ++tree_iter;
        ASSERT_NE(tree_iter, tree_iter_end);
        EXPECT_EQ(tree_iter.current_depth(), 2);
        EXPECT_EQ(tree_iter.lookup_index(), (std::vector<int>{1, 3}));
        ASSERT_TRUE(tree_iter->index().has_value());
        EXPECT_EQ(tree_iter->index().value(), 20);

        ++tree_iter;
        EXPECT_EQ(tree_iter, tree_iter_end);

    }
}