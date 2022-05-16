/**
 * symbol_tree_tests.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "gtest/gtest.h"

#include "symbol_tree_tests.h"

#include <vector>
#include <stack>

namespace NPATK::Tests {

    SymbolTree& SymbolTreeFixture::create_tree(std::initializer_list<SymbolPair> pairs) {
        SymbolSet ss{std::vector<SymbolPair>(pairs)};
        ss.pack();
        this->the_tree = std::make_unique<SymbolTree>(std::move(ss));
        return *this->the_tree;
    }

    void SymbolTreeFixture::compare_to(std::initializer_list<SymbolPair> pairs, bool only_topology) {
        SymbolSet ss{std::vector<SymbolPair>(pairs)};
        ss.pack();
        SymbolTree target_tree{std::move(ss)};
        compare_to(target_tree, only_topology);
    }

    void SymbolTreeFixture::compare_to(std::initializer_list<Symbol> extra, std::initializer_list<SymbolPair> pairs,
                                       bool only_topology) {
        SymbolSet ss{std::vector<Symbol>(extra), std::vector<SymbolPair>(pairs)};
        ss.pack();
        SymbolTree target_tree{std::move(ss)};
        compare_to(target_tree, only_topology);
    }

    void SymbolTreeFixture::compare_to(const SymbolTree &target_tree, bool only_topology) {

        ASSERT_TRUE(this->the_tree) << "Must instantiate source tree!";
        SymbolTree& test_tree = *(this->the_tree);

        // Trees must have same node count
        ASSERT_EQ(test_tree.count_nodes(), target_tree.count_nodes());

        for (size_t node_index = 0; node_index < test_tree.count_nodes(); ++node_index) {
            const auto& lhs_node = test_tree[node_index];
            const auto& rhs_node = target_tree[node_index];
            ASSERT_EQ(lhs_node.id, rhs_node.id) << "Nodes ids at index " << node_index << " must match";

            auto lhs_iter = lhs_node.begin();
            auto rhs_iter = rhs_node.begin();
            bool lhs_at_end = (lhs_iter == lhs_node.end());
            bool rhs_at_end = (rhs_iter == rhs_node.end());

            size_t child_index = 0;
            while (!lhs_at_end && !rhs_at_end) {
                ASSERT_NE(lhs_iter->origin, nullptr) << "Node: " << node_index << " Child: " << child_index;
                ASSERT_NE(rhs_iter->origin, nullptr) << "Node: " << node_index << " Child: " << child_index;
                ASSERT_NE(lhs_iter->target, nullptr) << "Node: " << node_index << " Child: " << child_index;
                ASSERT_NE(rhs_iter->target, nullptr) << "Node: " << node_index << " Child: " << child_index;
                EXPECT_EQ(lhs_iter->origin->id, rhs_iter->origin->id) << "Node: " << node_index << " Child: " << child_index;
                EXPECT_EQ(lhs_iter->target->id, rhs_iter->target->id) << "Node: " << node_index << " Child: " << child_index;
                if (!only_topology) {
                    EXPECT_EQ(lhs_iter->link_type, rhs_iter->link_type)
                                        << "Node: " << node_index << " Child: " << child_index;
                }

                // Advance iterators
                ++lhs_iter;
                ++rhs_iter;

                // Check if at end...
                lhs_at_end = (lhs_iter == lhs_node.end());
                rhs_at_end = (rhs_iter == rhs_node.end());
                ++child_index;
                //ASSERT_EQ(lhs_at_end, rhs_at_end) << "Iterators for node_index " << node_index << " must end at same point.";
            }

            ASSERT_TRUE(lhs_at_end) << "Iterators for node " << node_index
                                    << " must end at same point. Ended at child " << child_index;
            ASSERT_TRUE(rhs_at_end) << "Iterators for node " << node_index
                                    << " must end at same point. Ended at child " << child_index;


        }
    }

    TEST_F(SymbolTreeFixture, Create_EmptyTree) {
        auto& empty_tree = this->create_tree({});
        ASSERT_EQ(empty_tree.count_nodes(), 0) << "Empty tree has no nodes.";
        ASSERT_EQ(empty_tree.max_links(), 0) << "Empty tree has no links.";
    }

    TEST_F(SymbolTreeFixture, Create_OneLink) {
        auto& one_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}}});
        ASSERT_EQ(one_link.count_nodes(), 2) << "Tree has two nodes.";
        ASSERT_EQ(one_link.max_links(), 1) << "Tree has one link.";

        const auto& base_node = one_link[0];
        const auto& child_node = one_link[1];
        ASSERT_NE(&base_node, &child_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(child_node.id, 1);
        ASSERT_FALSE(base_node.empty()) << "Base node should not be empty.";
        ASSERT_TRUE(child_node.empty()) << "Child node should be empty.";

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


    TEST_F(SymbolTreeFixture, Create_OneRecursion) {
        auto& one_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{0}}});
        ASSERT_EQ(one_link.count_nodes(), 1) << "Tree has one node.";
        ASSERT_EQ(one_link.max_links(), 1) << "Tree has one link.";

        const auto& base_node = one_link[0];
        EXPECT_EQ(base_node.id, 0);
        ASSERT_FALSE(base_node.empty()) << "Node should not be empty.";

        // Test first link
        auto base_node_iter = base_node.begin();
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& first_link = *base_node_iter;
        ASSERT_EQ(first_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(first_link.target, &base_node) << "Link must target child.";

        // Only one link from base
        ++base_node_iter;
        ASSERT_EQ(base_node_iter, base_node.end()) << "Only one link from base node.";
    }


    TEST_F(SymbolTreeFixture, Create_OneLinkOneRecursion) {
        auto& one_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}}, SymbolPair{SymbolExpression{1}, SymbolExpression{1}}});
        ASSERT_EQ(one_link.count_nodes(), 2) << "Tree has two nodes.";
        ASSERT_EQ(one_link.max_links(), 2) << "Tree has two links.";

        const auto& base_node = one_link[0];
        const auto& child_node = one_link[1];
        ASSERT_NE(&base_node, &child_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(child_node.id, 1);
        ASSERT_FALSE(base_node.empty()) << "Base node should not be empty.";
        ASSERT_FALSE(child_node.empty()) << "Child node should not be empty.";

        // Test first link
        auto base_node_iter = base_node.begin();
        ASSERT_NE(base_node_iter, base_node.end()) << "Iterator must not be at end";
        const auto& first_link = *base_node_iter;
        ASSERT_EQ(first_link.origin, &base_node) << "Link must originate from base.";
        ASSERT_EQ(first_link.target, &child_node) << "Link must target child.";


        // Only one link from base
        ++base_node_iter;
        ASSERT_EQ(base_node_iter, base_node.end()) << "Only one link from base node.";

        // Test child node link
        auto child_node_iter = child_node.begin();
        ASSERT_NE(child_node_iter, child_node.end()) << "Child must have children.";
        const auto& second_link = *child_node_iter;
        ASSERT_EQ(second_link.origin, &child_node) << "Link must originate from child.";
        ASSERT_EQ(second_link.target, &child_node) << "Link must target child.";

        // Only one link originating from child
        ++child_node_iter;
        ASSERT_EQ(child_node_iter, child_node.end()) << "Only one link from child node.";
    }



    TEST_F(SymbolTreeFixture, Create_ChainLink) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}}, SymbolPair{SymbolExpression{1}, SymbolExpression{2}}});
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
        ASSERT_FALSE(base_node.empty()) << "Base node should not be empty.";
        ASSERT_FALSE(child_node.empty()) << "Child node should not be empty.";
        ASSERT_TRUE(grandchild_node.empty()) << "Grandchild node should be empty.";

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

    TEST_F(SymbolTreeFixture, Create_ChainLinkMiddleRecursion) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                      SymbolPair{SymbolExpression{1}, SymbolExpression{1}},
                                      SymbolPair{SymbolExpression{1}, SymbolExpression{2}}});
        ASSERT_EQ(chain_link.count_nodes(), 3) << "Tree has three nodes.";
        ASSERT_EQ(chain_link.max_links(), 3) << "Tree has three links.";

        const auto& base_node = chain_link[0];
        const auto& child_node = chain_link[1];
        const auto& grandchild_node = chain_link[2];
        ASSERT_NE(&base_node, &child_node) << "Nodes must not be same object!";
        ASSERT_NE(&base_node, &grandchild_node) << "Nodes must not be same object!";
        ASSERT_NE(&child_node, &grandchild_node) << "Nodes must not be same object!";
        EXPECT_EQ(base_node.id, 0);
        EXPECT_EQ(child_node.id, 1);
        EXPECT_EQ(grandchild_node.id, 2);
        ASSERT_FALSE(base_node.empty()) << "Base node should not be empty.";
        ASSERT_FALSE(child_node.empty()) << "Child node should not be empty.";
        ASSERT_TRUE(grandchild_node.empty()) << "Grandchild node should be empty.";

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
        ASSERT_EQ(second_link.origin, &child_node) << "Child link must originate from child.";
        ASSERT_EQ(second_link.target, &child_node) << "Child link must target child.";

        // Only one link originating from child
        ++child_node_iter;
        ASSERT_NE(child_node_iter, child_node.end()) << "Child must have second child.";
        const auto& third_link = *child_node_iter;
        ASSERT_EQ(third_link.origin, &child_node) << "Link must originate from base.";
        ASSERT_EQ(third_link.target, &grandchild_node) << "Link must target child.";

        // Only one link originating from child
        ++child_node_iter;
        ASSERT_EQ(child_node_iter, child_node.end()) << "Total two links from child node.";

        auto grandchild_node_iter = grandchild_node.begin();
        ASSERT_EQ(grandchild_node_iter, grandchild_node.end()) << "Grandchild has no children.";
    }


    TEST_F(SymbolTreeFixture, Create_OpenTriangle) {
        auto& open_tri = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}}, SymbolPair{SymbolExpression{0}, SymbolExpression{2}}});
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
        ASSERT_FALSE(base_node.empty()) << "Base node should not be empty.";
        ASSERT_TRUE(childA_node.empty()) << "ChildA node should be empty.";
        ASSERT_TRUE(childB_node.empty()) << "ChildB node should be empty.";

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

    TEST_F(SymbolTreeFixture, Create_ClosedTriangle) {
        auto& closed_tri = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                    SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                                    SymbolPair{SymbolExpression{1}, SymbolExpression{2}}});
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
        ASSERT_FALSE(base_node.empty()) << "Base node should not be empty.";
        ASSERT_FALSE(childA_node.empty()) << "ChildA node should not be empty.";
        ASSERT_TRUE(childB_node.empty()) << "ChildB node should be empty.";

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

    TEST_F(SymbolTreeFixture, Create_InverseTriangle) {
        auto& open_tri = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{2}}, SymbolPair{SymbolExpression{1}, SymbolExpression{2}}});
        ASSERT_EQ(open_tri.count_nodes(), 3) << "Tree has three nodes.";
        ASSERT_EQ(open_tri.max_links(), 2) << "Tree has two links.";

        const auto& baseA_node = open_tri[0];
        const auto& baseB_node = open_tri[1];
        const auto& child_node = open_tri[2];
        ASSERT_NE(&baseA_node, &child_node) << "Nodes must not be same object!";
        ASSERT_NE(&baseB_node, &child_node) << "Nodes must not be same object!";
        ASSERT_NE(&baseA_node, &baseB_node) << "Nodes must not be same object!";
        EXPECT_EQ(baseA_node.id, 0);
        EXPECT_EQ(baseB_node.id, 1);
        EXPECT_EQ(child_node.id, 2);
        ASSERT_FALSE(baseA_node.empty()) << "BaseA node should not be empty.";
        ASSERT_FALSE(baseB_node.empty()) << "BaseB node should not be empty.";
        ASSERT_TRUE(child_node.empty()) << "Child node should be empty.";

        // Test first link
        auto baseA_node_iter = baseA_node.begin();
        ASSERT_NE(baseA_node_iter, baseA_node.end()) << "Iterator must not be at end";
        const auto& first_link = *baseA_node_iter;
        ASSERT_EQ(first_link.origin, &baseA_node) << "Link must originate from baseA.";
        ASSERT_EQ(first_link.target, &child_node) << "Link must target child.";

        // No more links
        ++baseA_node_iter;
        ASSERT_EQ(baseA_node_iter, baseA_node.end()) << "Only one link from base A node.";

        // Test second link
        auto baseB_node_iter = baseB_node.begin();
        ASSERT_NE(baseB_node_iter, baseB_node.end()) << "Iterator must not be at end";
        const auto& second_link = *baseB_node_iter;
        ASSERT_EQ(second_link.origin, &baseB_node) << "Link must originate from baseA.";
        ASSERT_EQ(second_link.target, &child_node) << "Link must target child.";

        // No more links
        ++baseB_node_iter;
        ASSERT_EQ(baseB_node_iter, baseA_node.end()) << "Only one link from base A node.";

        // No links from child
        auto child_node_iter = child_node.begin();
        ASSERT_EQ(child_node_iter, child_node.end()) << "ChildB should have no children.";
    }



    TEST_F(SymbolTreeFixture, Simplify_OneRecursion) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{0}}});

        chain_link.simplify();
        this->compare_to({Symbol{0}}, {});

        EXPECT_FALSE(chain_link[0].is_zero());
        EXPECT_FALSE(chain_link[0].real_is_zero);
        EXPECT_FALSE(chain_link[0].im_is_zero);
    }

    TEST_F(SymbolTreeFixture, Simplify_ChainLink) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                              SymbolPair{SymbolExpression{1}, SymbolExpression{2}}});

        chain_link.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}}});

        EXPECT_FALSE(chain_link[0].is_zero());
        EXPECT_FALSE(chain_link[0].real_is_zero);
        EXPECT_FALSE(chain_link[0].im_is_zero);

    }

    TEST_F(SymbolTreeFixture, Simplify_Triangle) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                              SymbolPair{SymbolExpression{0}, SymbolExpression{2}}});

        chain_link.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}}});


        EXPECT_FALSE(chain_link[0].is_zero());
        EXPECT_FALSE(chain_link[0].real_is_zero);
        EXPECT_FALSE(chain_link[0].im_is_zero);
    }

    TEST_F(SymbolTreeFixture, Simplify_TriangleWithDescendents) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                              SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                                              SymbolPair{SymbolExpression{2}, SymbolExpression{3}},
                                              SymbolPair{SymbolExpression{2}, SymbolExpression{4}},
                                             });

        chain_link.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{3}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{4}},
                          });


        EXPECT_FALSE(chain_link[0].is_zero());
        EXPECT_FALSE(chain_link[0].real_is_zero);
        EXPECT_FALSE(chain_link[0].im_is_zero);

    }


    TEST_F(SymbolTreeFixture, Simplify_InverseTriangle) {
        auto &inverse_tri = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{2}}, SymbolPair{SymbolExpression{1}, SymbolExpression{2}}});
        inverse_tri.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}}});


        EXPECT_FALSE(inverse_tri[0].is_zero());
        EXPECT_FALSE(inverse_tri[0].real_is_zero);
        EXPECT_FALSE(inverse_tri[0].im_is_zero);

    }

    TEST_F(SymbolTreeFixture, Simplify_Diamond) {
        auto &diamond = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                           SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                                           SymbolPair{SymbolExpression{1}, SymbolExpression{3}},
                                           SymbolPair{SymbolExpression{2}, SymbolExpression{3}}
                                           });
        diamond.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{3}}});

        EXPECT_FALSE(diamond[0].is_zero());
        EXPECT_FALSE(diamond[0].real_is_zero);
        EXPECT_FALSE(diamond[0].im_is_zero);


    }

    TEST_F(SymbolTreeFixture, Simplify_BranchingZigZag) {
        auto &b_zz = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{3}},
                                           SymbolPair{SymbolExpression{1}, SymbolExpression{4}},
                                           SymbolPair{SymbolExpression{1}, SymbolExpression{5}},
                                           SymbolPair{SymbolExpression{1}, SymbolExpression{6}},
                                           SymbolPair{SymbolExpression{2}, SymbolExpression{3}},
                                           SymbolPair{SymbolExpression{2}, SymbolExpression{4}}
                                           });
        b_zz.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{3}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{4}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{5}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{6}},
                          });

        EXPECT_FALSE(b_zz[0].is_zero());
        EXPECT_FALSE(b_zz[0].real_is_zero);
        EXPECT_FALSE(b_zz[0].im_is_zero);

    }


    TEST_F(SymbolTreeFixture, SimplifyToZero_OneRecursion) {
        auto& onenull = this->create_tree({SymbolPair{0, 0, true, false}}); // 0 = -0

        onenull.simplify();
        this->compare_to({Symbol{0}}, {});

        EXPECT_TRUE(onenull[0].is_zero());
        EXPECT_TRUE(onenull[0].real_is_zero);
        EXPECT_TRUE(onenull[0].im_is_zero);
    }

    TEST_F(SymbolTreeFixture, SimplifyToZero_ChainRecursion) {
        auto& chain_link = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                             SymbolPair{SymbolExpression{1}, SymbolExpression{2}},
                                             SymbolPair{2, 2, true, false}}); // 2 = -2

        chain_link.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}}});

        EXPECT_TRUE(chain_link[0].is_zero());
        EXPECT_TRUE(chain_link[0].real_is_zero);
        EXPECT_TRUE(chain_link[0].im_is_zero);
    }


    TEST_F(SymbolTreeFixture, SimplifyToZero_Triangle) {
        auto& nulltri = this->create_tree({SymbolPair{0, 1, true, false},  // 0 = -1
                                              SymbolPair{0, 2, true, false},  // 0 = -2
                                              SymbolPair{1, 2, true, false}}); // 1 = -2

        nulltri.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}}},
                         true);

        EXPECT_TRUE(nulltri[0].is_zero());
        EXPECT_TRUE(nulltri[0].real_is_zero);
        EXPECT_TRUE(nulltri[0].im_is_zero);
    }


    TEST_F(SymbolTreeFixture, SimplifyToZero_Diamond) {
        auto &diamond = this->create_tree({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                                           SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                                           SymbolPair{SymbolExpression{1}, SymbolExpression{3}},
                                           SymbolPair{2,3, true, false}
                                          });
        diamond.simplify();
        this->compare_to({SymbolPair{SymbolExpression{0}, SymbolExpression{1}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{2}},
                          SymbolPair{SymbolExpression{0}, SymbolExpression{3}}}, true);

        EXPECT_TRUE(diamond[0].is_zero());
        EXPECT_TRUE(diamond[0].real_is_zero);
        EXPECT_TRUE(diamond[0].im_is_zero);

    }



    TEST_F(SymbolTreeFixture, Substitute_Triangle) {
        auto& tree = this->create_tree({SymbolPair{SymbolExpression{10}, SymbolExpression{20}},
                                              SymbolPair{SymbolExpression{10}, SymbolExpression{-30}}});

        tree.simplify();
        this->compare_to({SymbolPair{SymbolExpression{10}, SymbolExpression{20}},
                          SymbolPair{SymbolExpression{10}, SymbolExpression{-30}}});

        auto expr_a = tree.substitute(SymbolExpression{20});
        EXPECT_EQ(expr_a.id, 10);
        EXPECT_EQ(expr_a.negated, false);
        EXPECT_EQ(expr_a.conjugated, false);

        auto expr_b = tree.substitute(SymbolExpression{30});
        EXPECT_EQ(expr_b.id, 10);
        EXPECT_EQ(expr_b.negated, true);
        EXPECT_EQ(expr_b.conjugated, false);

        auto expr_c = tree.substitute(SymbolExpression{-30});
        EXPECT_EQ(expr_c.id, 10);
        EXPECT_EQ(expr_c.negated, false);
        EXPECT_EQ(expr_c.conjugated, false);

    }

}