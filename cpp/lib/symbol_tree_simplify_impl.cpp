/**
 * symbol_tree_simplify_impl.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol.h"
#include "symbol_tree_simplify_impl.h"

#include <stack>

namespace NPATK::detail {

    void SymbolNodeSimplifyImpl::simplify() {

        // If already has canonical origin, node has been visited already
        if (this_node.canonical_origin != nullptr) {
            return;
        }

        // If node has no children, nothing to do
        if (this_node.empty()) {
            return;
        }

        // See if any children think they are already part of a tree
        std::vector<RebaseInfoImpl> nodes_to_rebase;
        size_t lowest_node_found_index = this->find_already_linked(nodes_to_rebase);

        // Anything to rebase?
        SymbolTree::SymbolLink * descendent_hint = nullptr;
        SymbolTree::SymbolLink * link_for_base = nullptr;
        if (!nodes_to_rebase.empty()) {
            auto& pivot_entry = nodes_to_rebase[lowest_node_found_index];
            auto& canonical_node = *(pivot_entry.linkFromCanonicalNode->origin);

            for (auto& move_entry : nodes_to_rebase) {
                auto& move_node = *(move_entry.linkToMove->target);

                if (move_entry.is_pivot) {
                    // Pivot node already has correct canonical base, and has no children...

                    // Link to base can be undone, and reset to new information
                    move_entry.linkToMove->detach_and_reset();
                    link_for_base = move_entry.linkToMove;
                    link_for_base->link_type = compose(move_entry.linkFromCanonicalNode->link_type,
                                                       move_entry.relationToBase);
                    link_for_base->target = &this_node;

                } else {
                    assert(move_entry.linkToMove != nullptr);
                    auto& move_link = *(move_entry.linkToMove);

                    // Repurpose link from base -...-> move, to be link from canon -...-> move
                    move_link.detach();
                    move_link.link_type = move_entry.relationToCanonical;
                    canonical_node.subsume(&move_link);
                }
            }

            // Finally, move base node into canonical structure.
            assert(link_for_base != nullptr);
            // We are guaranteed not to be a duplicate entry in the canonical node,
            //  but the same is not guaranteed for our descendents.
            auto [did_merge, next_desc_hint] = canonical_node.insert_ordered(link_for_base);
            descendent_hint = next_desc_hint;
        }

        // Now, only "unvisited" children remain.
        if (descendent_hint == nullptr) {
            this->incorporate_all_descendents();
        } else {
            // TODO: same thing, but relocated descendents elsewhere!
        }

    }

    namespace {
        struct NodeAndIter {
            SymbolTree::SymbolNode *node;
            SymbolTree::SymbolLinkIterator iter;
            EqualityType relationToBase = EqualityType::none;

            constexpr explicit NodeAndIter(SymbolTree::SymbolNode *the_node, EqualityType rtb) noexcept
                    : node(the_node), iter(the_node->begin()), relationToBase(rtb) {}

        };
    }

    size_t SymbolNodeSimplifyImpl::find_already_linked(std::vector<RebaseInfoImpl>& rebase_list) {
        rebase_list.clear();

        size_t lowest_node_found_index = -1;
        symbol_name_t lowest_node_found = std::numeric_limits<symbol_name_t>::max();

        // Scan children; kinda recursively
        SymbolTree::SymbolNode * node_cursor = &this_node;
        std::stack<NodeAndIter> recurse_stack{};
        recurse_stack.emplace(node_cursor, EqualityType::equal);
        while (!recurse_stack.empty()) {
            auto& stack_frame = recurse_stack.top();

            // Node has no more children,
            if (stack_frame.iter == stack_frame.node->end()) {
                // Go up one level in the stack.
                recurse_stack.pop();
                if (recurse_stack.empty()) {
                    break;
                }
                // Advance iterator below
                node_cursor = recurse_stack.top().node;
                ++(recurse_stack.top().iter);
                continue;
            }

            // Node still has children
            auto& current_link  = *(stack_frame.iter);

            // Node's child has canonical origin, no need to traverse deeper...!
            if (current_link.target->canonical_origin != nullptr) {

                // Register link!
                rebase_list.emplace_back(RebaseInfoImpl{&current_link,
                                                        current_link.target->canonical_origin,
                                                        compose(stack_frame.relationToBase, current_link.link_type) });
                if (current_link.target->canonical_origin->origin->id < lowest_node_found) {
                    lowest_node_found = current_link.target->canonical_origin->origin->id;
                    lowest_node_found_index = rebase_list.size()-1;
                }

                // No need to look at children; all will be moved anyway.

                // Advance current iterator to next child, and continue
                ++(stack_frame.iter);
                continue;
            }

            // Node's child has children
            if (!current_link.target->empty()) {
                // Check not recursive
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
            auto &pivot_entry = rebase_list[lowest_node_found_index];

            // Find relationship between points
            for (auto &entry : rebase_list) {
                if (&entry == &pivot_entry) {
                    entry.is_pivot = true;
                    entry.relationToCanonical = entry.linkFromCanonicalNode->link_type;
                } else {
                    entry.is_pivot = false;
                    EqualityType relationToPivot = compose(pivot_entry.relationToBase, entry.relationToBase);
                    entry.relationToCanonical = compose(pivot_entry.linkFromCanonicalNode->link_type, relationToPivot);
                }
            }
        }

        return lowest_node_found_index;
    }


    namespace {
        struct MoveStack {
            SymbolTree::SymbolNode *node;
            SymbolTree::SymbolLink *cursor;
            EqualityType relationToBase = EqualityType::none;

            constexpr  MoveStack(SymbolTree::SymbolNode *the_node,
                                 EqualityType rtb) noexcept
                    : node(the_node),
                      cursor(the_node->empty() ? nullptr : &(*(the_node->begin()))),
                      relationToBase(rtb) { }

        };
    }


    void SymbolNodeSimplifyImpl::incorporate_all_descendents() {

        // TODO: Constraints (real=0, im=0, etc.)

        // Scan children; kinda recursively
        //SymbolTree::SymbolNode * node_cursor = &this_node;
        std::stack<MoveStack> recurse_stack{};
        recurse_stack.emplace(&this_node, EqualityType::equal);

        while (!recurse_stack.empty()) {
            auto& stack_frame = recurse_stack.top();

            // Node has no more children,
            if (stack_frame.cursor == nullptr) {
                // Go up one level in the stack.
                recurse_stack.pop();
                if (recurse_stack.empty()) {
                    break;
                }

                // Advance iterator below
                assert(recurse_stack.top().cursor != nullptr);
                SymbolTree::SymbolLink * canonical_link = recurse_stack.top().cursor;

                // If rebase is required:
                if (canonical_link->origin != &this_node) {
                    canonical_link->detach();
                    canonical_link->origin = &this_node;
                    canonical_link->link_type = recurse_stack.top().relationToBase;
                    auto [did_merge, inserted_link] = this_node.insert_ordered(canonical_link, nullptr);
                    recurse_stack.top().node->canonical_origin = inserted_link;
                    // Now, canonical_link is displaced
                    if (did_merge) {
                        // TODO: Special handling for merge?
                    }
                    // TODO: Handling of hints
                }
                recurse_stack.top().cursor = recurse_stack.top().cursor->next;

                continue;
            }

            // Node still has children
            auto& current_link = *(stack_frame.cursor);

            // Node's child has children
            if (!current_link.target->empty()) {

                // Check not recursive
                if (current_link.target != current_link.origin) {
                    // Go down one level in the stack
                    recurse_stack.emplace(current_link.target,
                                          compose(stack_frame.relationToBase, current_link.link_type));
                    continue;
                }

                // Resolve link to self, with nullity etc.
                auto [re_is_zero, im_is_zero] = implies_zero(current_link.link_type);
                stack_frame.node->real_is_zero |= re_is_zero;
                stack_frame.node->im_is_zero |= im_is_zero;

                // Propagate nullity down to base node
                this_node.real_is_zero |= stack_frame.node->real_is_zero;
                this_node.im_is_zero |= stack_frame.node->im_is_zero;

                // Delete link, and advance
                auto [prev, next] = current_link.detach_and_reset();
                stack_frame.cursor = next;
                continue;

            }

            // Otherwise, advance current iterator
            stack_frame.cursor = stack_frame.cursor->next;


        }

    }
}