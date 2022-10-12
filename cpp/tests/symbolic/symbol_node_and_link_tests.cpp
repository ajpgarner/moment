/**
 * symbol_node_and_link_tests.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 *
 * Tests for minor functions within SymbolTree::SymbolNode and SymbolTree::SymbolLink classes
 *
 */
#include "gtest/gtest.h"
#include "symbolic/symbol_tree.h"

namespace NPATK::Tests {

    class MockTree : public SymbolTree {
    public:
        MockTree() : SymbolTree{} {

        }
    };


    TEST(SymbolNodeAndLink, InsertBack_AtoB) {
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolLink linkToB{tree, &nodeB, EqualityType::equal};
        nodeA.insert_back(&linkToB);

        ASSERT_EQ(linkToB.origin, &nodeA) << "Origin should be node A";
        ASSERT_EQ(linkToB.target, &nodeB) << "Target should be node B";
        EXPECT_EQ(linkToB.link_type, EqualityType::equal);

        ASSERT_FALSE(nodeA.empty()) << "Node A should not be empty";
        ASSERT_TRUE(nodeB.empty()) << "Node B should be empty";
    }


    TEST(SymbolNodeAndLink, InsertBack_AtoBandC) {
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolNode nodeC{tree, 2};
        SymbolTree::SymbolLink linkB{tree, &nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &nodeC, EqualityType::equal};
        nodeA.insert_back(&linkB);
        nodeA.insert_back(&linkC);

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
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        auto iter = nodeA.begin();
        ASSERT_EQ(iter, nodeA.end());
    }

    TEST(SymbolNodeAndLink, TestIterator) {
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolNode nodeC{tree, 2};
        SymbolTree::SymbolLink linkB{tree, &nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &nodeC, EqualityType::equal};
        nodeA.insert_back(&linkB);
        nodeA.insert_back(&linkC);

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
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolNode nodeC{tree, 2};
        SymbolTree::SymbolLink linkB{tree, &nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &nodeC, EqualityType::equal};
        nodeA.insert_back(&linkB);
        nodeA.insert_back(&linkC);

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
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolLink link{tree, &nodeB, EqualityType::equal};
        nodeA.insert_back(&link);

        auto [prev, next] = link.detach_and_reset();
        EXPECT_EQ(prev, nullptr);
        EXPECT_EQ(next, nullptr);
        ASSERT_TRUE(nodeA.empty());

        auto iter = nodeA.cbegin();
        ASSERT_EQ(iter, nodeA.cend());
    }

    TEST(SymbolNodeAndLink, Unlink_FirstOfTwo) {
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolNode nodeC{tree, 2};
        SymbolTree::SymbolLink linkB{tree, &nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &nodeC, EqualityType::equal};
        nodeA.insert_back(&linkB);
        nodeA.insert_back(&linkC);


        auto [prev, next] = linkB.detach_and_reset();
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
        MockTree tree{};

        SymbolTree::SymbolNode nodeA{tree, 0};
        SymbolTree::SymbolNode nodeB{tree, 1};
        SymbolTree::SymbolNode nodeC{tree, 2};
        SymbolTree::SymbolLink linkB{tree, &nodeB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &nodeC, EqualityType::equal};
        nodeA.insert_back(&linkB);
        nodeA.insert_back(&linkC);


        auto [prev, next] = linkC.detach_and_reset();
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
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 1};
        SymbolTree::SymbolNode childB{tree, 2};
        SymbolTree::SymbolNode childC{tree, 2};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{tree, &childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &childC, EqualityType::equal};
        base.insert_back(&linkA);
        base.insert_back(&linkB);
        base.insert_back(&linkC);

        auto [prev, next] = linkB.detach_and_reset();
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


    TEST(SymbolNodeAndLink, InsertOrdered_EmptyList) {
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 10};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};

        ASSERT_TRUE(base.empty());
        auto [did_merge, insA] = base.insert_ordered(&linkA);
        ASSERT_FALSE(base.empty());
        EXPECT_FALSE(did_merge);
        EXPECT_EQ(insA, &linkA);

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must point first to link A";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }


    TEST(SymbolNodeAndLink, InsertOrdered_FrontNoHint) {
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 10};
        SymbolTree::SymbolNode childB{tree, 20};
        SymbolTree::SymbolNode childC{tree, 30};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{tree, &childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &childC, EqualityType::equal};
        base.insert_back(&linkA);
        base.insert_back(&linkB);
        base.insert_back(&linkC);

        SymbolTree::SymbolNode testNode{tree, 5};
        SymbolTree::SymbolLink testLink{tree, &testNode, EqualityType::negated};
        auto [did_merge, insTest] = base.insert_ordered(&testLink);

        EXPECT_FALSE(did_merge);
        EXPECT_EQ(insTest, &testLink);

        ASSERT_FALSE(base.empty());

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &testLink) << "Iter must point first to test node link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must then point to child A link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkB) << "Iter must then point to child B link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must then point to child C link";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }

    TEST(SymbolNodeAndLink, InsertOrdered_MiddleNoHint) {
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 10};
        SymbolTree::SymbolNode childB{tree, 20};
        SymbolTree::SymbolNode childC{tree, 30};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{tree, &childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &childC, EqualityType::equal};
        base.insert_back(&linkA);
        base.insert_back(&linkB);
        base.insert_back(&linkC);

        SymbolTree::SymbolNode testNode{tree, 15};
        SymbolTree::SymbolLink testLink{tree, &testNode, EqualityType::negated};
        auto [did_merge, insTest] = base.insert_ordered(&testLink);

        EXPECT_FALSE(did_merge);
        EXPECT_EQ(insTest, &testLink);

        ASSERT_FALSE(base.empty());

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must first point to child A link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &testLink) << "Iter must then point to test node link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkB) << "Iter must then point to child B link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must then point to child C link";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }

    TEST(SymbolNodeAndLink, InsertOrdered_EndNoHint) {
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 10};
        SymbolTree::SymbolNode childB{tree, 20};
        SymbolTree::SymbolNode childC{tree, 30};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{tree, &childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &childC, EqualityType::equal};
        base.insert_back(&linkA);
        base.insert_back(&linkB);
        base.insert_back(&linkC);

        SymbolTree::SymbolNode testNode{tree, 35};
        SymbolTree::SymbolLink testLink{tree, &testNode, EqualityType::negated};
        auto [did_merge, insTest] = base.insert_ordered(&testLink);

        EXPECT_FALSE(did_merge);
        EXPECT_EQ(insTest, &testLink);

        ASSERT_FALSE(base.empty());

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must first point to child A link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkB) << "Iter must then point to child B link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must then point to child C link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &testLink) << "Iter must then point to test node link";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }

    TEST(SymbolNodeAndLink, InsertOrdered_MiddleWithHint) {
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 10};
        SymbolTree::SymbolNode childB{tree, 20};
        SymbolTree::SymbolNode childC{tree, 30};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{tree, &childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &childC, EqualityType::equal};
        base.insert_back(&linkA);
        base.insert_back(&linkB);
        base.insert_back(&linkC);

        SymbolTree::SymbolNode testNode{tree, 15};
        SymbolTree::SymbolLink testLink{tree, &testNode, EqualityType::negated};
        auto [did_merge, insTest] = base.insert_ordered(&testLink, &linkB);
        EXPECT_FALSE(did_merge);
        EXPECT_EQ(insTest, &testLink);

        ASSERT_FALSE(base.empty());

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must first point to child A link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &testLink) << "Iter must then point to test node link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkB) << "Iter must then point to child B link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must then point to child C link";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }

    TEST(SymbolNodeAndLink, InsertOrdered_EndWithHint) {
        MockTree tree{};

        SymbolTree::SymbolNode base{tree, 0};
        SymbolTree::SymbolNode childA{tree, 10};
        SymbolTree::SymbolNode childB{tree, 20};
        SymbolTree::SymbolNode childC{tree, 30};
        SymbolTree::SymbolLink linkA{tree, &childA, EqualityType::equal};
        SymbolTree::SymbolLink linkB{tree, &childB, EqualityType::equal};
        SymbolTree::SymbolLink linkC{tree, &childC, EqualityType::equal};
        base.insert_back(&linkA);
        base.insert_back(&linkB);
        base.insert_back(&linkC);

        SymbolTree::SymbolNode testNode{tree, 35};
        SymbolTree::SymbolLink testLink{tree, &testNode, EqualityType::negated};
        auto [did_merge, insTest] = base.insert_ordered(&testLink, &linkC);
        EXPECT_FALSE(did_merge);
        EXPECT_EQ(insTest, &testLink);

        ASSERT_FALSE(base.empty());

        auto iter = base.cbegin();
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkA) << "Iter must first point to child A link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkB) << "Iter must then point to child B link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &linkC) << "Iter must then point to child C link";

        ++iter;
        ASSERT_NE(iter, base.cend());
        EXPECT_EQ(&(*iter), &testLink) << "Iter must then point to test node link";

        ++iter;
        ASSERT_EQ(iter, base.cend());
    }

    TEST(SymbolNodeAndLink, Subsume_Concatenate) {
        MockTree tree{};

        SymbolTree::SymbolNode baseA{tree, 0};
        SymbolTree::SymbolNode childAA{tree, 10};
        SymbolTree::SymbolNode childAB{tree, 20};
        SymbolTree::SymbolNode childAC{tree, 30};
        SymbolTree::SymbolLink linkAA{tree, &childAA, EqualityType::equal};
        SymbolTree::SymbolLink linkAB{tree, &childAB, EqualityType::equal};
        SymbolTree::SymbolLink linkAC{tree, &childAC, EqualityType::equal};
        baseA.insert_back(&linkAA);
        baseA.insert_back(&linkAB);
        baseA.insert_back(&linkAC);

        SymbolTree::SymbolNode baseB{tree, 40};
        SymbolTree::SymbolNode childBA{tree, 50};
        SymbolTree::SymbolNode childBB{tree, 60};
        SymbolTree::SymbolNode childBC{tree, 70};
        SymbolTree::SymbolLink linkBA{tree, &childBA, EqualityType::equal};
        SymbolTree::SymbolLink linkBB{tree, &childBB, EqualityType::equal};
        SymbolTree::SymbolLink linkBC{tree, &childBC, EqualityType::equal};
        baseB.insert_back(&linkBA);
        baseB.insert_back(&linkBB);
        baseB.insert_back(&linkBC);

        SymbolTree::SymbolLink linkInB{tree, &baseB, EqualityType::negated};

        size_t ss_count = baseA.subsume(&linkInB);
        ASSERT_EQ(ss_count, 4) << "Four elements should have been added";
        ASSERT_FALSE(baseA.empty());

        auto iter = baseA.cbegin();
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAA) << "Iter must first point to link A.A";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAA) << "Link target must point to child A.A";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAB) << "Iter must first point to link A.B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAB) << "Link target must point to child A.B";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAC) << "Iter must then point to link A.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAC) << "Link target must point to child A.C";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkInB) << "Iter must then point to link to base B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &baseB) << "Link target must point to base B";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBA) << "Iter must then point to link B.A";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBA) << "Link target must point to child B.A";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBB) << "Iter must then point to link A.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBB) << "Link target must point to child B.B";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBC) << "Iter must then point to link B.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBC) << "Link target must point to child B.C";
        EXPECT_EQ(iter->link_type, EqualityType::negated);


        ++iter;
        ASSERT_EQ(iter, baseA.cend());

        ASSERT_TRUE(baseB.empty()) << "Base B must no longer have children.";
    }

    TEST(SymbolNodeAndLink, Subsume_Prefix) {
        MockTree tree{};

        SymbolTree::SymbolNode baseA{tree, 0};
        SymbolTree::SymbolNode childAA{tree, 50};
        SymbolTree::SymbolNode childAB{tree, 60};
        SymbolTree::SymbolNode childAC{tree, 70};
        SymbolTree::SymbolLink linkAA{tree, &childAA, EqualityType::equal};
        SymbolTree::SymbolLink linkAB{tree, &childAB, EqualityType::equal};
        SymbolTree::SymbolLink linkAC{tree, &childAC, EqualityType::equal};
        baseA.insert_back(&linkAA);
        baseA.insert_back(&linkAB);
        baseA.insert_back(&linkAC);

        SymbolTree::SymbolNode baseB{tree, 10};
        SymbolTree::SymbolNode childBA{tree, 20};
        SymbolTree::SymbolNode childBB{tree, 30};
        SymbolTree::SymbolNode childBC{tree, 40};
        SymbolTree::SymbolLink linkBA{tree, &childBA, EqualityType::equal};
        SymbolTree::SymbolLink linkBB{tree, &childBB, EqualityType::equal};
        SymbolTree::SymbolLink linkBC{tree, &childBC, EqualityType::equal};
        baseB.insert_back(&linkBA);
        baseB.insert_back(&linkBB);
        baseB.insert_back(&linkBC);

        SymbolTree::SymbolLink linkInB{tree, &baseB, EqualityType::negated};

        size_t ss_count = baseA.subsume(&linkInB);
        ASSERT_EQ(ss_count, 4) << "Four elements should have been added";
        ASSERT_FALSE(baseA.empty());

        auto iter = baseA.cbegin();
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkInB) << "Iter must first point to link to base B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &baseB) << "Link target must point to base B";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBA) << "Iter must then point to link B.A";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBA) << "Link target must point to child B.A";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBB) << "Iter must then point to link B.B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBB) << "Link target must point to child B.B";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBC) << "Iter must then point to link B.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBC) << "Link target must point to child B.C";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAA) << "Iter must then point to link A.A";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAA) << "Link target must point to child A.A";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAB) << "Iter must first point to link A.B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAB) << "Link target must point to child A.B";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAC) << "Iter must then point to link A.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAC) << "Link target must point to child A.C";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_EQ(iter, baseA.cend());

        ASSERT_TRUE(baseB.empty()) << "Base B must no longer have children.";
    }


    TEST(SymbolNodeAndLink, Subsume_Interleave) {
        MockTree tree{};

        SymbolTree::SymbolNode baseA{tree, 0};
        SymbolTree::SymbolNode childAA{tree, 30};
        SymbolTree::SymbolNode childAB{tree, 50};
        SymbolTree::SymbolNode childAC{tree, 70};
        SymbolTree::SymbolLink linkAA{tree, &childAA, EqualityType::equal};
        SymbolTree::SymbolLink linkAB{tree, &childAB, EqualityType::equal};
        SymbolTree::SymbolLink linkAC{tree, &childAC, EqualityType::equal};
        baseA.insert_back(&linkAA);
        baseA.insert_back(&linkAB);
        baseA.insert_back(&linkAC);

        SymbolTree::SymbolNode baseB{tree, 10};
        SymbolTree::SymbolNode childBA{tree, 40};
        SymbolTree::SymbolNode childBB{tree, 60};
        SymbolTree::SymbolNode childBC{tree, 80};
        SymbolTree::SymbolLink linkBA{tree, &childBA, EqualityType::equal};
        SymbolTree::SymbolLink linkBB{tree, &childBB, EqualityType::equal};
        SymbolTree::SymbolLink linkBC{tree, &childBC, EqualityType::equal};
        baseB.insert_back(&linkBA);
        baseB.insert_back(&linkBB);
        baseB.insert_back(&linkBC);

        SymbolTree::SymbolLink linkInB{tree, &baseB, EqualityType::negated};

        size_t ss_count = baseA.subsume(&linkInB);
        ASSERT_EQ(ss_count, 4) << "Four elements should have been added";
        ASSERT_FALSE(baseA.empty());

        auto iter = baseA.cbegin();
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkInB) << "Iter must first point to link to base B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &baseB) << "Link target must point to base B";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAA) << "Iter must then point to link A.A";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAA) << "Link target must point to child A.A";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBA) << "Iter must then point to link B.A";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBA) << "Link target must point to child B.A";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAB) << "Iter must first point to link A.B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAB) << "Link target must point to child A.B";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBB) << "Iter must then point to link B.B";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBB) << "Link target must point to child B.B";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkAC) << "Iter must then point to link A.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childAC) << "Link target must point to child A.C";
        EXPECT_EQ(iter->link_type, EqualityType::equal);

        ++iter;
        ASSERT_NE(iter, baseA.cend());
        EXPECT_EQ(&(*iter), &linkBC) << "Iter must then point to link B.C";
        EXPECT_EQ(iter->origin, &baseA) << "Link origin must point to base A";
        EXPECT_EQ(iter->target, &childBC) << "Link target must point to child B.C";
        EXPECT_EQ(iter->link_type, EqualityType::negated);

        ++iter;
        ASSERT_EQ(iter, baseA.cend());

        ASSERT_TRUE(baseB.empty()) << "Base B must no longer have children.";
    }

}