/**
 * symbol_tree_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "symbol_tree.h"

namespace NPATK::Tests {

    namespace {
        SymbolTree createTree(std::initializer_list<SymbolPair> pairs) {
            SymbolSet ss{std::vector<SymbolPair>(pairs)};
            ss.pack();
            return SymbolTree{ss};
        }
    }

    TEST(SymbolTree, Create_EmptyTree) {
        auto empty_tree = createTree({});
        ASSERT_EQ(empty_tree.count_nodes(), 0) << "Empty tree has no nodes.";
        ASSERT_EQ(empty_tree.max_links(), 0) << "Empty tree has no links.";
    }

    TEST(SymbolTree, Create_OneLink) {
        auto one_link = createTree({SymbolPair{Symbol{0}, Symbol{1}}});
        ASSERT_EQ(one_link.count_nodes(), 2) << "Tree has two nodes.";
        ASSERT_EQ(one_link.max_links(), 1) << "Tree has one link.";

        const auto& base_node = one_link[0];
        const auto& child_node = one_link[1];
        ASSERT_NE(&base_node, &child_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(child_node.id, 1);

        // Test first link
        auto base_node_iter = base_node.begin();
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& first_link = *base_node_iter;
        ASSERT_EQ(first_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(first_link.target, &child_node) << "Link must target child.";

        // Only one link from base
        ++base_node_iter;
        ASSERT_EQ(base_node_iter, base_node.end()) << "Only one link from base node.";

        // No links from child
        auto child_node_iter = child_node.begin();
        ASSERT_EQ(child_node_iter, child_node.end()) << "Child should have no children.";
    }

    TEST(SymbolTree, Create_ChainLink) {
        auto chain_link = createTree({SymbolPair{Symbol{0}, Symbol{1}}, SymbolPair{Symbol{1}, Symbol{2}}});
        ASSERT_EQ(chain_link.count_nodes(), 3) << "Tree has three nodes.";
        ASSERT_EQ(chain_link.max_links(), 2) << "Tree has two links.";

        const auto& base_node = chain_link[0];
        const auto& child_node = chain_link[1];
        const auto& grandchild_node = chain_link[2];
        ASSERT_NE(&base_node, &child_node) << "Nodes must not be same object!";
        ASSERT_NE(&base_node, &grandchild_node) << "Nodes must not be same object!";
        ASSERT_NE(&child_node, &grandchild_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(child_node.id, 1);
        EXPECT_EQ(grandchild_node.id, 2);

        // Test first link
        auto base_node_iter = base_node.begin();
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& first_link = *base_node_iter;
        ASSERT_EQ(first_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(first_link.target, &child_node) << "Link must target child.";

        // Only one link originating from base
        ++base_node_iter;
        ASSERT_EQ(base_node_iter, base_node.end()) << "Only one link from base node.";

        // Test child node link
        auto child_node_iter = child_node.begin();
        ASSERT_NE(child_node_iter, child_node.end()) << "Child must have children.";
        const auto& second_link = *child_node_iter;
        ASSERT_EQ(second_link.origin, &child_node) << "Link must originate from base.";
        ASSERT_EQ(second_link.target, &grandchild_node) << "Link must target child.";

        // Only one link originating from child
        ++child_node_iter;
        ASSERT_EQ(child_node_iter, child_node.end()) << "Only one link from child node.";

        auto grandchild_node_iter = grandchild_node.begin();
        ASSERT_EQ(grandchild_node_iter, grandchild_node.end()) << "Grandchild has no children.";
    }


    TEST(SymbolTree, Create_OpenTriangle) {
        auto open_tri = createTree({SymbolPair{Symbol{0}, Symbol{1}}, SymbolPair{Symbol{0}, Symbol{2}}});
        ASSERT_EQ(open_tri.count_nodes(), 3) << "Tree has three nodes.";
        ASSERT_EQ(open_tri.max_links(), 2) << "Tree has two links.";

        const auto& base_node = open_tri[0];
        const auto& childA_node = open_tri[1];
        const auto& childB_node = open_tri[2];
        ASSERT_NE(&base_node, &childA_node) << "Nodes must not be same object!";
        ASSERT_NE(&base_node, &childB_node) << "Nodes must not be same object!";
        ASSERT_NE(&childA_node, &childB_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(childA_node.id, 1);
        EXPECT_EQ(childB_node.id, 2);

        // Test first link
        auto base_node_iter = base_node.begin();
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& first_link = *base_node_iter;
        ASSERT_EQ(first_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(first_link.target, &childA_node) << "Link must target childA.";

        // Test second link
        ++base_node_iter;
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& second_link = *base_node_iter;
        ASSERT_EQ(second_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(second_link.target, &childB_node) << "Link must target childB.";

        // No more links
        ++base_node_iter;
        ASSERT_EQ(base_node_iter, base_node.end()) << "Only two links from base node.";

        // No links from children
        auto childA_node_iter = childA_node.begin();
        ASSERT_EQ(childA_node_iter, childA_node.end()) << "ChildA should have no children.";

        auto childB_node_iter = childB_node.begin();
        ASSERT_EQ(childB_node_iter, childB_node.end()) << "ChildB should have no children.";
    }

    TEST(SymbolTree, Create_ClosedTriangle) {
        auto closed_tri = createTree({SymbolPair{Symbol{0}, Symbol{1}},
                                    SymbolPair{Symbol{0}, Symbol{2}},
                                    SymbolPair{Symbol{1}, Symbol{2}}});
        ASSERT_EQ(closed_tri.count_nodes(), 3) << "Tree has three nodes.";
        ASSERT_EQ(closed_tri.max_links(), 3) << "Tree has three links.";

        const auto& base_node = closed_tri[0];
        const auto& childA_node = closed_tri[1];
        const auto& childB_node = closed_tri[2];
        ASSERT_NE(&base_node, &childA_node) << "Nodes must not be same object!";
        ASSERT_NE(&base_node, &childB_node) << "Nodes must not be same object!";
        ASSERT_NE(&childA_node, &childB_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(childA_node.id, 1);
        EXPECT_EQ(childB_node.id, 2);


        // Test first link
        auto base_node_iter = base_node.begin();
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& first_link = *base_node_iter;
        ASSERT_EQ(first_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(first_link.target, &childA_node) << "Link must target childA.";

        // Test second link
        ++base_node_iter;
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& second_link = *base_node_iter;
        ASSERT_EQ(second_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(second_link.target, &childB_node) << "Link must target childB.";

        // No more links
        ++base_node_iter;
        ASSERT_EQ(base_node_iter, base_node.end()) << "Only two links from base node.";

        // Child A should have a link
        auto childA_node_iter = childA_node.begin();
        ASSERT_NE(childA_node_iter, childA_node.end()) << "ChildA should have children.";
        const auto& sibling_link = *childA_node_iter;
        ASSERT_EQ(sibling_link.origin, &childA_node) << "Link must originate from childA.";
        ASSERT_EQ(sibling_link.target, &childB_node) << "Link must target childB.";

        // Child A has only one link
        ++childA_node_iter;
        ASSERT_EQ(childA_node_iter, childA_node.end()) << "ChildA should have just one child.";

        // No links from child B
        auto childB_node_iter = childB_node.begin();
        ASSERT_EQ(childB_node_iter, childB_node.end()) << "ChildB should have no children.";
    }

}