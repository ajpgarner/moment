/**
 * index_tree_tests.cpp
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
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
}