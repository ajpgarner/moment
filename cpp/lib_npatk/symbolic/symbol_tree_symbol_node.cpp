/**
 * symbol_tree_symbol_node.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "symbol_tree.h"

#include <stack>

namespace NPATK {

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

    void SymbolTree::SymbolNode::insert_back(SymbolTree::SymbolLink* link) noexcept {
        if (this->last_link != nullptr) {
            this->last_link->next = link;
            link->prev = this->last_link;
            link->next = nullptr;
            this->last_link = link;
        } else {
            this->first_link = link;
            this->last_link = link;
        }
        link->origin = this;
    }

    std::pair<bool, SymbolTree::SymbolLink*>
    SymbolTree::SymbolNode::insert_ordered(SymbolTree::SymbolLink *link, SymbolTree::SymbolLink * hint) noexcept {
        // Debug assertions:
        assert(link->origin == nullptr);
        assert(link->prev == nullptr);
        assert(link->next == nullptr);

        link->origin = this;

        // If only link in node:
        if (this->empty()) {
            this->first_link = link;
            this->last_link = link;
            link->prev = nullptr;
            link->next = nullptr;
            return {false, link};
        }

        // Node not empty, so guaranteed this->first_link / this->last_link != nullptr

        // No hint provided; so start from first link of Node
        if (hint == nullptr) {
            hint = this->first_link;
        }

        while (hint != nullptr) {
            assert(link != hint);
            assert(link->target != nullptr);
            assert(hint->target != nullptr);

            if (link->target->id < hint->target->id) {
                link->prev = hint->prev; // might be nullptr
                link->next = hint;
                if (hint == this->first_link) {
                    assert(hint->prev == nullptr);
                    this->first_link = link;
                }
                if (link->prev != nullptr) {
                    hint->prev->next = link;
                }
                hint->prev = link;
                return {false, link};
            }
            if (link->target->id == hint->target->id) {
                // Merge, by combining link types
                hint->link_type |= link->link_type;

                // See if merge changes nullity of symbol
                auto [implies_re_zero, implies_im_zero] = implies_zero(hint->link_type);
                this->real_is_zero = this->real_is_zero || implies_re_zero;
                this->im_is_zero = this->im_is_zero || implies_im_zero;

                // Input link is reset and effectively orphaned, as it should not point to anything

                link->origin = nullptr;
                link->target = nullptr;
                link->link_type = EqualityType::none;
                // XXx tree->release()? somewhere??

                // Return ptr to link already in the list
                return {true, hint};
            }
            hint = hint->next;
        }

        // Did not insert yet, so put at end of list:
        this->last_link->next = link;
        link->prev = this->last_link;
        link->next = nullptr;
        this->last_link = link;

        return {false, link};
    }


    size_t SymbolTree::SymbolNode::subsume(SymbolTree::SymbolLink *source) noexcept {
        assert(source->target != nullptr);

        size_t count = 1;
        SymbolNode& sourceNode = *(source->target);
        const EqualityType baseET = source->link_type;

        // First, insert source node
        auto [did_merge_source, hint] = this->insert_ordered(source);

        sourceNode.canonical_origin = source;
        sourceNode.im_is_zero = sourceNode.im_is_zero || this->im_is_zero;
        sourceNode.real_is_zero = sourceNode.real_is_zero || this->real_is_zero;

        // Now, insert all sub-children.
        SymbolLink * source_ptr = sourceNode.first_link;
        while (source_ptr != nullptr) {
            SymbolLink * next_ptr = source_ptr->next; // might be nullptr

            // Crude detach, as we will reset whole chain...
            auto& linkToMove = *source_ptr;
            linkToMove.next = nullptr;
            linkToMove.prev = nullptr;
            linkToMove.origin = nullptr;
            linkToMove.link_type = compose(baseET, linkToMove.link_type);

            assert(linkToMove.target != nullptr);
            auto& childNode = *(linkToMove.target);
            childNode.canonical_origin = &linkToMove;
            childNode.im_is_zero = childNode.im_is_zero || this->im_is_zero;
            childNode.real_is_zero = childNode.real_is_zero || this->real_is_zero;

            auto [did_merge, next_hint] = this->insert_ordered(&linkToMove, hint);
            hint = next_hint;
            source_ptr = next_ptr;
            ++count;
        }

        // Source no longer has children
        sourceNode.first_link = nullptr;
        sourceNode.last_link = nullptr;

        return count;
    }



    void SymbolTree::SymbolNode::simplify() {
        // If node already has canonical origin, node has been visited already (and cannot simplify itself further).
        if (this->canonical_origin != nullptr) {
            return;
        }

        // If node has no children, nothing to simplify
        if (this->empty()) {
            return;
        }

        // See if any children think they are already part of a tree
        std::vector<RebaseInfoImpl> nodes_to_rebase{};
        size_t lowest_node_found_index = find_already_linked(nodes_to_rebase);

        // Anything to rebase?
        if (!nodes_to_rebase.empty()) {
            rebase_nodes(nodes_to_rebase, lowest_node_found_index);

            // Now, only "unvisited" children remain, and they should be moved via pivot to new canonical base
            auto &pivot_entry = nodes_to_rebase[lowest_node_found_index];
            auto &canonical_node = *(pivot_entry.linkFromCanonicalNode->origin);
            incorporate_all_descendents(&canonical_node,
                                        compose(pivot_entry.relationToBase, pivot_entry.relationToCanonical));

        } else {
            // Nothing to rebase, but move this node's descendents to this node
            incorporate_all_descendents(this, EqualityType::equal);
        }
    }



    size_t SymbolTree::SymbolNode::find_already_linked(std::vector<RebaseInfoImpl>& rebase_list) {
        rebase_list.clear();
        std::map<SymbolLink*, size_t> rebase_alias;

        size_t lowest_node_found_index = std::numeric_limits<size_t>::max();
        symbol_name_t lowest_node_found = std::numeric_limits<symbol_name_t>::max();

        // Scan node and children recursively
        std::stack<NodeAndIter> recurse_stack{};
        recurse_stack.emplace(this, EqualityType::equal);
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
                assert(current_link.target->canonical_origin->origin != this);

                EqualityType full_relation = compose(stack_frame.relationToBase, current_link.link_type);


                // Have we seen this link before?
                auto existing_link = rebase_alias.find(&current_link);
                if (existing_link != rebase_alias.end()) {
                    // If so, add to existing rebase, but incorporate possible different equality type.
                    rebase_list[existing_link->second].relationToBase |= full_relation;

                } else {
                    // Register link!
                    rebase_list.emplace_back(RebaseInfoImpl{&current_link,
                                                            current_link.target->canonical_origin,
                                                            full_relation});

                    // Don't let us include the same link twice...
                    rebase_alias.emplace(std::make_pair(&current_link, rebase_list.size()-1));

                    if (current_link.target->canonical_origin->origin->id < lowest_node_found) {
                        lowest_node_found = current_link.target->canonical_origin->origin->id;
                        lowest_node_found_index = rebase_list.size() - 1;
                    }
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

    void SymbolTree::SymbolNode::rebase_nodes(std::vector<RebaseInfoImpl> &nodes_to_rebase,
                                              size_t lowest_node_found_index) {

        auto& pivot_entry = nodes_to_rebase[lowest_node_found_index];
        assert(pivot_entry.pivot_status == RebaseInfoImpl::PivotStatus::Pivot);

        auto& canonical_node = *(pivot_entry.linkFromCanonicalNode->origin);
        EqualityType link_to_base_type = compose(pivot_entry.linkFromCanonicalNode->link_type,
                                                 pivot_entry.relationToBase);

        for (auto& move_entry : nodes_to_rebase) {
            assert(move_entry.linkToMove != nullptr);
            auto& move_link = *(move_entry.linkToMove);

            assert(move_link.target != nullptr);
            auto& move_node = *(move_link.target);

            if (move_entry.pivot_status == RebaseInfoImpl::PivotStatus::Pivot) {
                // Pivot node already has correct canonical, and (by virtue of knowing the canonical) has no children...
                assert(move_entry.linkFromCanonicalNode->origin == &canonical_node);
                assert(move_node.empty());

                // Can remove the link between base and this pivot
                move_entry.linkToMove->detach_and_reset();
                this->the_tree.release_link(move_entry.linkToMove);
            } else if (move_entry.pivot_status == RebaseInfoImpl::PivotStatus::FalsePivot) {
                // Node has correct canonical, but is not the first node with this status...
                assert(move_entry.linkFromCanonicalNode->origin == &canonical_node);
                assert(move_entry.linkFromCanonicalNode != move_entry.linkToMove);

                // Having a canonical at all, should have no children...
                if (!move_node.empty()) {
                    throw;
                }
                assert(move_node.empty());

                // Update canonical link with new equality type information (i.e. for purpose of detecting zeros)
                move_entry.linkFromCanonicalNode->merge_in(move_entry.relationToCanonical);

                // Redundant link can then be detached and freed
                move_entry.linkToMove->detach_and_reset();
                this->the_tree.release_link(move_entry.linkToMove);

            } else {
                assert(move_entry.linkToMove->target->canonical_origin != nullptr);
                auto& previous_canonical_link = *(move_entry.linkToMove->target->canonical_origin);
                auto& previous_canonical_node = *(previous_canonical_link.origin);
                assert(&move_link != &previous_canonical_link);

                // Does node have correct canonical node (because it's been updated elsewhere in the loop)?
                if (&canonical_node == &previous_canonical_node) {
                    // Merge, and check nullity
                    move_node.canonical_origin->merge_in(move_entry.relationToCanonical);

                    // Redundant link can then be detached and freed
                    move_entry.linkToMove->detach_and_reset();
                    this->the_tree.release_link(move_entry.linkToMove);
                    continue;
                }

                assert(&canonical_node != &previous_canonical_node);

                // Repurpose link '[base -> ...? ] -> found node', to be link  'canon -...-> move's previous canon'
                move_link.detach();
                move_link.link_type = compose(move_entry.relationToCanonical, previous_canonical_link.link_type);
                move_link.target = &previous_canonical_node;

                canonical_node.subsume(&move_link);
            }
        }

        // Finally, move base node into canonical structure.
        // We are guaranteed not to be a duplicate entry in the canonical node (even if not true for descendents).
        auto * link_for_base = this->the_tree.getAvailableLink();
        link_for_base->target = this;
        link_for_base->link_type = link_to_base_type;

        auto [did_merge, descendent_hint] = canonical_node.insert_ordered(link_for_base);
        assert(!did_merge);
        assert(descendent_hint == link_for_base);
        this->canonical_origin = link_for_base;
    }

    void SymbolTree::SymbolNode::incorporate_all_descendents(SymbolTree::SymbolNode * rebase_node,
                                                             EqualityType base_et) {

        // Iterate recursively through tree, acting on parent nodes before their children
        std::stack<MoveStack> recurse_stack{};
        recurse_stack.emplace(this, base_et);

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
                EqualityType inSituType = current_link.link_type;
                current_link.link_type = compose(stack_frame.relationToBase, current_link.link_type);

                auto [did_merge, inserted_link] = rebase_node->insert_ordered(&current_link, stack_frame.hint);
                stack_frame.hint = inserted_link;

                // Redundant link, free:
                if (did_merge) {
                    this->the_tree.release_link(&current_link);
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
                    // Go down one level in the stack (composing link, as originally found)
                    recurse_stack.emplace(inserted_link->target,
                                          compose(stack_frame.relationToBase, inSituType));
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
                this->the_tree.release_link(&current_link);
            }
        }
    }

    SymbolExpression SymbolTree::SymbolNode::canonical_expression() const noexcept {
        // Return canonical id, maybe with negation or conjugation
        if (this->canonical_origin != nullptr) {
            return SymbolExpression{this->canonical_origin->origin->id,
                                    is_negated(this->canonical_origin->link_type),
                                    is_conjugated(this->canonical_origin->link_type)};
        }
        // Return self id (no negation or conjugation)
        return SymbolExpression{this->id};
    }

    SymbolPair SymbolTree::SymbolNode::canonical_pair() const noexcept {
        if (this->canonical_origin == nullptr) {
            return SymbolPair{this->id, this->id, false, false};
        }
        return SymbolPair{this->id, this->canonical_origin->origin->id,
                          is_negated(this->canonical_origin->link_type),
                          is_conjugated(this->canonical_origin->link_type)};
    }
}