/**
 * symbol_node_and_link_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Tests for minor functions within SymbolTree::SymbolNode and SymbolTree::SymbolLink classes
 *
 */
#include "gtest/gtest.h"
#include "symbol_tree.h"

namespace NPATK::Tests {


    TEST(SymbolNodeAndLink, LinkBack_AtoB) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolLink linkToB{&nodeB, EqualityType::equal};
        nodeA.link_back(&linkToB);

        ASSERT_EQ(linkToB.origin, &nodeA) << "Origin should be node A";
        ASSERT_EQ(linkToB.target, &nodeB) << "Target should be node B";
        EXPECT_EQ(linkToB.link_type, EqualityType::equal);

        ASSERT_FALSE(nodeA.empty()) << "Node A should not be empty";
        ASSERT_TRUE(nodeB.empty()) << "Node B should be empty";
    }


    TEST(SymbolNodeAndLink, LinkBack_AtoBandC) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolNode nodeC{2};
        SymbolTree::SymbolLink linkB{&nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{&nodeC, EqualityType::equal};
        nodeA.link_back(&linkB);
        nodeA.link_back(&linkC);

        ASSERT_EQ(linkB.origin, &nodeA) << "Origin of linkB should be node A";
        ASSERT_EQ(linkB.target, &nodeB) << "Target of linkB should be node B";
        EXPECT_EQ(linkB.link_type, EqualityType::equal);

        ASSERT_EQ(linkB.origin, &nodeA) << "Origin of linkB should be node A";
        ASSERT_EQ(linkB.target, &nodeB) << "Target of linkC should be node C";
        EXPECT_EQ(linkB.link_type, EqualityType::equal);

        ASSERT_FALSE(nodeA.empty()) << "Node A should not be empty";
        ASSERT_TRUE(nodeB.empty()) << "Node B should be empty";
        ASSERT_TRUE(nodeB.empty()) << "Node C should be empty";
    }


    TEST(SymbolNodeAndLink, TestEmptyIterator) {

        SymbolTree::SymbolNode nodeA{0};
        auto iter = nodeA.begin();
        ASSERT_EQ(iter, nodeA.end());
    }

    TEST(SymbolNodeAndLink, TestIterator) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolNode nodeC{2};
        SymbolTree::SymbolLink linkB{&nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{&nodeC, EqualityType::equal};
        nodeA.link_back(&linkB);
        nodeA.link_back(&linkC);

        auto iter = nodeA.begin();
        ASSERT_NE(iter, nodeA.end());
        ASSERT_EQ(&linkB, &(*iter)) << "Iter must point first to link B";
        ++iter;
        ASSERT_NE(iter, nodeA.end());
        ASSERT_EQ(&linkC, &(*iter)) << "Iter must point next to link C";
        ++iter;

        ASSERT_EQ(iter, nodeA.end());
    }

    TEST(SymbolNodeAndLink, TestConstIterator) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolNode nodeC{2};
        SymbolTree::SymbolLink linkB{&nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{&nodeC, EqualityType::equal};
        nodeA.link_back(&linkB);
        nodeA.link_back(&linkC);

        auto iter = nodeA.cbegin();
        ASSERT_NE(iter, nodeA.cend());
        ASSERT_EQ(&(*iter), &linkB) << "Iter must point first to link B";
        ++iter;
        ASSERT_NE(iter, nodeA.cend());
        ASSERT_EQ(&(*iter), &linkC) << "Iter must point next to link C";
        ++iter;

        ASSERT_EQ(iter, nodeA.cend());
    }


    TEST(SymbolNodeAndLink, Unlink_OnlyLink) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolLink link{&nodeB, EqualityType::equal};
        nodeA.link_back(&link);

        auto [prev, next] = link.unlink_and_reset();
        EXPECT_EQ(prev, nullptr);
        EXPECT_EQ(next, nullptr);
        ASSERT_TRUE(nodeA.empty());

        auto iter = nodeA.cbegin();
        ASSERT_EQ(iter, nodeA.cend());
    }

    TEST(SymbolNodeAndLink, Unlink_FirstOfTwo) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolNode nodeC{2};
        SymbolTree::SymbolLink linkB{&nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{&nodeC, EqualityType::equal};
        nodeA.link_back(&linkB);
        nodeA.link_back(&linkC);


        auto [prev, next] = linkB.unlink_and_reset();
        EXPECT_EQ(prev, nullptr);
        EXPECT_EQ(next, &linkC);
        ASSERT_FALSE(nodeA.empty());

        auto iter = nodeA.cbegin();
        ASSERT_NE(iter, nodeA.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must point first to link C";

        ++iter;
        ASSERT_EQ(iter, nodeA.cend());
    }

    TEST(SymbolNodeAndLink, Unlink_SecondOfTwo) {

        SymbolTree::SymbolNode nodeA{0};
        SymbolTree::SymbolNode nodeB{1};
        SymbolTree::SymbolNode nodeC{2};
        SymbolTree::SymbolLink linkB{&nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{&nodeC, EqualityType::equal};
        nodeA.link_back(&linkB);
        nodeA.link_back(&linkC);


        auto [prev, next] = linkC.unlink_and_reset();
        EXPECT_EQ(prev, &linkB);
        EXPECT_EQ(next, nullptr);
        ASSERT_FALSE(nodeA.empty());

        auto iter = nodeA.cbegin();
        ASSERT_NE(iter, nodeA.cend());
        EXPECT_EQ(&(*iter), &linkB) << "Iter must point first to link B";

        ++iter;
        ASSERT_EQ(iter, nodeA.cend());
    }


    TEST(SymbolNodeAndLink, Unlink_SecondOfThree) {

        SymbolTree::SymbolNode base{0};
        SymbolTree::SymbolNode childA{1};
        SymbolTree::SymbolNode childB{2};
        SymbolTree::SymbolNode childC{2};
        SymbolTree::SymbolLink linkA{&childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{&childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{&childC, EqualityType::equal};
        base.link_back(&linkA);
        base.link_back(&linkB);
        base.link_back(&linkC);

        auto [prev, next] = linkB.unlink_and_reset();
        EXPECT_EQ(prev, &linkA);
        EXPECT_EQ(next, &linkC);
        ASSERT_FALSE(base.empty());

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must point first to link A";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must next point to link C";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }

}