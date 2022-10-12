/**
 * symbol_tree_simplify_impl.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol.h"
#include "symbol_tree_simplify_impl.h"

#include <stack>

namespace NPATK::detail {

    namespace {
        /** Represents a node, and an iterator over the node's children */
        struct NodeAndIter {
            SymbolTree::SymbolNode *node;
            SymbolTree::SymbolLinkIterator iter;
            EqualityType relationToBase = EqualityType::none;

            constexpr explicit NodeAndIter(SymbolTree::SymbolNode *the_node, EqualityType rtb) noexcept
                    : node(the_node), iter(the_node->begin()), relationToBase(rtb) {}

            /** True if iter is at its end */
            [[nodiscard]] bool done() const noexcept { return this->iter == this->node->end(); }

        };

        struct MoveStack {
            SymbolTree::SymbolNode * node;
            SymbolTree::SymbolLink * cursor;
            SymbolTree::SymbolLink * hint = nullptr;

            EqualityType relationToBase = EqualityType::none;

            constexpr  MoveStack(SymbolTree::SymbolNode *the_node,
                                 EqualityType rtb) noexcept
                    : node(the_node),
                      cursor(the_node->empty() ? nullptr : &(*(the_node->begin()))),
                      relationToBase(rtb) { }

        };
    }

    void SymbolNodeSimplifyImpl::simplify() {
        // Simplify
        const size_t symbol_count = theTree.tree_nodes.size();
        for (symbol_name_t node_id = 0; node_id < symbol_count; ++node_id) {
            simplifyNode(node_id);
        }

        // Propagate real & imaginary nullity
        propagate_nullity();

        // Nodes that are formally zero should alias to zero
        sweep_zero();

        // Count aliases
        count_noncanonical_nodes();
    }

    size_t SymbolNodeSimplifyImpl::count_noncanonical_nodes() {
        theTree.num_aliases = 0;
        for (auto& node : theTree.tree_nodes) {
            if (node.canonical_origin != nullptr) {
                ++theTree.num_aliases;
            }
        }
        return theTree.num_aliases;
    }

    void SymbolNodeSimplifyImpl::sweep_zero() {
        const size_t symbol_count = theTree.count_nodes();

        auto &zeroNode = theTree.tree_nodes[0];
        for (symbol_name_t node_id = 1; node_id < symbol_count; ++node_id) {
            auto &theNode = theTree.tree_nodes[node_id];

            // Early exit on nodes that have a parent
            if (theNode.canonical_origin != nullptr) {
                continue;
            }

            if (theNode.is_zero()) {
                auto * newLink = theTree.getAvailableLink();
                assert(newLink != nullptr);
                newLink->link_type = EqualityType::equal;
                newLink->target = &theNode;
                zeroNode.subsume(newLink);
            }
        }
    }

    void SymbolNodeSimplifyImpl::propagate_nullity() {
        for (auto& node : theTree.tree_nodes) {
            if (node.canonical_origin != nullptr) {
                continue;
            }

            for (auto& child_link : node) {
                auto& child = child_link.target;
                // Assert that children should not have nullity that the parent does not already have.
                assert(!(child->real_is_zero && (!node.real_is_zero)));
                assert(!(child->im_is_zero && (!node.im_is_zero)));

                child->real_is_zero = node.real_is_zero;
                child->im_is_zero = node.im_is_zero;

                // Simplify link relationships based on symbol properties
                if (node.is_zero()) {
                    child_link.link_type = EqualityType::equal;
                } else if (node.real_is_zero) {
                    child_link.link_type = simplifyPureImaginary(child_link.link_type);
                } else if (node.im_is_zero) {
                    child_link.link_type = simplifyPureReal(child_link.link_type);
                }
            }
        }
    }


    void SymbolNodeSimplifyImpl::simplifyNode(size_t node_id) {
        assert(node_id < theTree.tree_nodes.size());
        auto this_node = &(theTree.tree_nodes[node_id]);

        // If node already has canonical origin, node has been visited already (and cannot simplify itself further).
        if (this_node->canonical_origin != nullptr) {
            return;
        }

        // If node has no children, nothing to simplify
        if (this_node->empty()) {
            return;
        }

        // See if any children think they are already part of a tree
        std::vector<RebaseInfoImpl> nodes_to_rebase;
        size_t lowest_node_found_index = find_already_linked(this_node, nodes_to_rebase);

        // Anything to rebase?
        if (!nodes_to_rebase.empty()) {
            rebaseNodes(this_node, nodes_to_rebase, lowest_node_found_index);

            // Now, only "unvisited" children remain, and they should be moved via pivot to new canonical base
            auto &pivot_entry = nodes_to_rebase[lowest_node_found_index];
            auto &canonical_node = *(pivot_entry.linkFromCanonicalNode->origin);
            incorporate_all_descendents(this_node, &canonical_node,
                                        compose(pivot_entry.relationToBase, pivot_entry.relationToCanonical));

        } else {
            // Nothing to rebase, but move this node's descendents to this node
            incorporate_all_descendents(this_node, this_node, EqualityType::equal);
        }
    }

    size_t SymbolNodeSimplifyImpl::find_already_linked(SymbolTree::SymbolNode * const base_node,
                                                       std::vector<RebaseInfoImpl>& rebase_list) {
        rebase_list.clear();

        size_t lowest_node_found_index = std::numeric_limits<size_t>::max();
        symbol_name_t lowest_node_found = std::numeric_limits<symbol_name_t>::max();

        // Scan node and children recursively
        std::stack<NodeAndIter> recurse_stack{};
        recurse_stack.emplace(base_node, EqualityType::equal);
        while (!recurse_stack.empty()) {
            auto& stack_frame = recurse_stack.top();

            // If node has no more children...
            if (stack_frame.done()) {
                // Go up one level in the stack.
                recurse_stack.pop(); // [invalidates stack_frame]
                if (recurse_stack.empty()) {
                    // Nothing more to do, exit loop
                    break;
                }

                // Otherwise, advance iterator of new top node of stack
                ++(recurse_stack.top().iter);
                continue;
            }

            // Node still has children; look at link from node to next child
            auto& current_link  = *(stack_frame.iter);
            assert(current_link.origin == stack_frame.node);

            // If node's child has canonical origin...!
            if (current_link.target->canonical_origin != nullptr) {
                // Origin should not be base node...!
                assert(current_link.target->canonical_origin->origin != base_node);

                // Register link!
                rebase_list.emplace_back(RebaseInfoImpl{&current_link,
                                                        current_link.target->canonical_origin,
                                                        compose(stack_frame.relationToBase, current_link.link_type)});
                if (current_link.target->canonical_origin->origin->id < lowest_node_found) {
                    lowest_node_found = current_link.target->canonical_origin->origin->id;
                    lowest_node_found_index = rebase_list.size()-1;
                }

                // No need to look at (grand)children; all will be moved anyway!
                // Instead, advance current iterator to next child, and continue loop.
                ++(stack_frame.iter);
                continue;
            }

            // If node's child has children...
            if (!current_link.target->empty()) {
                // Check that link isn't directly to self...
                if (current_link.target != current_link.origin) {
                    // Go down one level in the stack
                    recurse_stack.emplace(current_link.target,
                                          compose(stack_frame.relationToBase, current_link.link_type));
                    continue;
                }
            }

            // Otherwise, advance current iterator
            ++(stack_frame.iter);
        }

        // If non trivial list found...
        if (!rebase_list.empty()) {
            const auto &pivot_entry = rebase_list[lowest_node_found_index];

            // Find relationship between points
            bool assigned_pivot = false;
            for (size_t index = 0; index < rebase_list.size(); ++index) {
                auto &entry = rebase_list[index];
                if (index == lowest_node_found_index) {
                    entry.pivot_status = RebaseInfoImpl::PivotStatus::Pivot;
                    entry.relationToCanonical = entry.linkFromCanonicalNode->link_type;
                } else {
                    EqualityType relationToPivot = compose(pivot_entry.relationToBase, entry.relationToBase);
                    EqualityType relationToCanon = compose(pivot_entry.linkFromCanonicalNode->link_type,
                                                          relationToPivot);

                    // Node is not the pivot, but still has correct canonical.
                    if (entry.linkFromCanonicalNode->origin == pivot_entry.linkFromCanonicalNode->origin) {
                        entry.pivot_status = RebaseInfoImpl::PivotStatus::FalsePivot;
                        // Two paths to canonical node: merge link types
                        entry.relationToCanonical = entry.linkFromCanonicalNode->link_type | relationToCanon;
                    } else {
                        entry.pivot_status = RebaseInfoImpl::PivotStatus::NotPivot;
                        entry.relationToCanonical = relationToCanon;
                    }
                }
            }
        }

        return lowest_node_found_index;
    }

    SymbolTree::SymbolLink *
    SymbolNodeSimplifyImpl::rebaseNodes(SymbolTree::SymbolNode * const base_node,
                                       std::vector<RebaseInfoImpl> &nodes_to_rebase,
                                       size_t lowest_node_found_index) {

        /* The link we will use ultimately for attaching the base node to the canonical node */
        SymbolTree::SymbolLink * link_for_base = nullptr;

        SymbolTree::SymbolLink * descendent_hint;
        auto& pivot_entry = nodes_to_rebase[lowest_node_found_index];
        assert(pivot_entry.pivot_status == RebaseInfoImpl::PivotStatus::Pivot);

        auto& canonical_node = *(pivot_entry.linkFromCanonicalNode->origin);

        for (auto& move_entry : nodes_to_rebase) {
            assert(move_entry.linkToMove != nullptr);
            assert(move_entry.linkToMove->target != nullptr);
            auto& move_node = *(move_entry.linkToMove->target);

            if (move_entry.pivot_status == RebaseInfoImpl::PivotStatus::Pivot) {
                // Pivot node already has correct canonical, and (by virtue of knowing the canonical) has no children...
                assert(move_entry.linkFromCanonicalNode->origin == &canonical_node);

                // Can remove the link between base and this pivot
                move_entry.linkToMove->detach_and_reset();

                // Memory location for this link can now be used for link from canonical to base:
                link_for_base = move_entry.linkToMove;
                link_for_base->link_type = compose(move_entry.linkFromCanonicalNode->link_type,
                                                   move_entry.relationToBase);
                link_for_base->target = base_node; // no origin, or children, yet...
            } else if (move_entry.pivot_status == RebaseInfoImpl::PivotStatus::FalsePivot) {
                // Node has correct canonical, but is not the first node with this status...
                assert(move_entry.linkFromCanonicalNode->origin == &canonical_node);

                // Update canonical link with new equality type information (i.e. for purpose of detecting zeros)
                move_entry.linkFromCanonicalNode->link_type = move_entry.relationToCanonical;

                // Redundant link can then be detached and freed
                move_entry.linkToMove->detach_and_reset();
                this->theTree.releaseLink(move_entry.linkToMove);
            } else {
                assert(move_entry.linkToMove->target->canonical_origin != nullptr);
                auto& move_link = *(move_entry.linkToMove);
                auto& previous_canonical_link = *(move_entry.linkToMove->target->canonical_origin);
                assert(&move_link != &previous_canonical_link);
                auto& previous_canonical_node = *(previous_canonical_link.origin);
                assert(&canonical_node != &previous_canonical_node);

                // Repurpose link '[base -> ...? ] -> found node', to be link  'canon -...-> move's previous canon'
                move_link.detach();
                move_link.link_type = compose(move_entry.relationToCanonical, previous_canonical_link.link_type);
                move_link.target = &previous_canonical_node;

                canonical_node.subsume(&move_link);
            }
        }

        // Finally, move base node into canonical structure.
        assert(link_for_base != nullptr);
        base_node->canonical_origin = link_for_base;

        // We are guaranteed not to be a duplicate entry in the canonical node,
        //  but the same is not guaranteed for our descendents.
        auto [did_merge, next_desc_hint] = canonical_node.insert_ordered(link_for_base);
        descendent_hint = next_desc_hint;
        return descendent_hint;
    }

    void SymbolNodeSimplifyImpl::incorporate_all_descendents(SymbolTree::SymbolNode * base_node,
                                                             SymbolTree::SymbolNode * rebase_node,
                                                             EqualityType base_et) {

        // Iterate recursively through tree, acting on parent nodes before their children
        std::stack<MoveStack> recurse_stack{};
        recurse_stack.emplace(base_node, base_et);

        while (!recurse_stack.empty()) {
            auto& stack_frame = recurse_stack.top();

            // Node has no more children...
            if (stack_frame.cursor == nullptr) {
                // Ascend one level in the stack.
                recurse_stack.pop();
                if (recurse_stack.empty()) {
                    break;
                }
                continue;
            }

            auto& current_link = *(stack_frame.cursor);

            // If not recursive, reinsert as child of canonical node...
            if (current_link.target != current_link.origin) {
                auto [prev, next_child] = current_link.detach();

                // Recalculate link type...
                current_link.link_type = compose(stack_frame.relationToBase, current_link.link_type);

                auto [did_merge, inserted_link] = rebase_node->insert_ordered(&current_link, stack_frame.hint);
                stack_frame.hint = inserted_link;

                // Redundant link, free:
                if (did_merge) {
                    theTree.releaseLink(&current_link);
                }

                inserted_link->target->canonical_origin = inserted_link;

                // Test nullity, and propagate downwards
                auto [re_is_zero, im_is_zero] = inserted_link->implies_zero();
                re_is_zero = re_is_zero || stack_frame.node->real_is_zero || rebase_node->real_is_zero;
                im_is_zero = im_is_zero || stack_frame.node->im_is_zero || rebase_node->im_is_zero;
                stack_frame.node->real_is_zero = rebase_node->real_is_zero = re_is_zero;
                stack_frame.node->im_is_zero = rebase_node->im_is_zero = im_is_zero;

                // See if node has children, and if so, descend one level
                if (!inserted_link->target->empty()) {
                    // Go down one level in the stack
                    recurse_stack.emplace(inserted_link->target,
                                          compose(stack_frame.relationToBase, inserted_link->link_type));
                }
                stack_frame.cursor = next_child;

            } else { // link is reflexively from and to same node.
                // Test nullity, and propagate
                auto [re_is_zero, im_is_zero] = current_link.implies_zero();
                re_is_zero = re_is_zero || stack_frame.node->real_is_zero || rebase_node->real_is_zero;
                im_is_zero = im_is_zero || stack_frame.node->im_is_zero || rebase_node->im_is_zero;
                stack_frame.node->real_is_zero = rebase_node->real_is_zero = re_is_zero;
                stack_frame.node->im_is_zero = rebase_node->im_is_zero = im_is_zero;

                // Reset link:
                auto [prev, next_child] = current_link.detach_and_reset();
                stack_frame.cursor = next_child;
                theTree.releaseLink(&current_link);
            }
        }
    }


}