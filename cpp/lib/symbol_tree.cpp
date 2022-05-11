/**
 * symbol_tree.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_tree.h"

#include <iostream>
#include <stack>

namespace NPATK {
    SymbolTree::SymbolTree(const SymbolSet &symbols) {

        /** Create nodes */
        size_t symbol_count = symbols.symbol_count();
        this->tree_nodes.reserve(symbol_count);
        for (symbol_name_t i = 0; i < symbol_count; ++i) {
            this->tree_nodes.emplace_back(i);
        }

        /** Create links */
        this->tree_links.reserve(symbols.link_count());

        size_t insert_index = 0;

        for (const auto [key, link_type] : symbols ) {
            SymbolNode * source_node = &this->tree_nodes[key.first];
            SymbolNode * target_node = &this->tree_nodes[key.second];

            this->tree_links.emplace_back(target_node, link_type);
            SymbolLink * this_link = &this->tree_links[insert_index];
            source_node->link_back(this_link);

            ++insert_index;
        }
    }

    void SymbolTree::SymbolNode::link_back(SymbolTree::SymbolLink* link) noexcept {
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

    std::ostream& operator<<(std::ostream& os, const SymbolTree& st) {
        for (auto symbol : st.tree_nodes) {
            os << symbol.id;

            bool once = false;

            for (const auto& lsl : symbol) {
                 if (once) {
                     os << ",\t";
                 } else {
                     os << "\t->\t";
                 }

                 os << lsl.target->id << "[";

                 if ((lsl.link_type & EqualityType::equal) == EqualityType::equal) {
                     os << "=";
                 }
                 if ((lsl.link_type & EqualityType::negated) == EqualityType::negated) {
                     os << "-";
                 }
                 if ((lsl.link_type & EqualityType::conjugated) == EqualityType::conjugated) {
                     os << "*";
                 }
                 if ((lsl.link_type & EqualityType::neg_conj) == EqualityType::neg_conj) {
                     os << "x";
                 }
                 os << "]";

                 once = true;
             }


            os << "\n";
        }
        return os;
    }


    void SymbolTree::simplify() {
        const size_t symbol_count = this->tree_nodes.size();
        for (symbol_name_t base_id = 0; base_id < symbol_count; ++base_id) {
            this->tree_nodes[base_id].relink();
        }
    }

    std::pair<SymbolTree::SymbolLink *, SymbolTree::SymbolLink *>
            SymbolTree::SymbolLink::unlink_and_reset() noexcept {
        auto old_values = std::make_pair(this->prev, this->next);

        if (this->prev != nullptr) {
            this->prev->next = this->next; // Might be nullptr
        } else if (this->origin != nullptr) {
            // No previous nodes, this means first link in origin list needs updating
            this->origin->first_link = this->next; // Might be nullptr
        }

        if (this->next != nullptr) {
            this->next->prev = old_values.first; // Might be nullptr
        } else if (this->origin != nullptr) {
            // No succeeding nodes, this means last link in origin list needs updating
            this->origin->last_link = old_values.first; // Might be nullptr
        }

        // Detach, reset.
        this->origin = nullptr;
        this->target = nullptr;
        this->prev = nullptr;
        this->next = nullptr;
        this->link_type = EqualityType::none;

        return old_values;
    }


    void SymbolTree::SymbolNode::relink() {
        // If already has canonical origin, node has been visited already
        if (this->canonical_origin != nullptr) {
          return;
        }

        // If node has no children, nothing to do
        if (this->empty()) {
            return;
        }

        // See if any children think they are already part of a tree
        std::vector<RebaseInfoImpl> nodes_to_rebase;
        size_t lowest_node_found_index = this->find_already_linked(nodes_to_rebase);

        // Anything to rebase?


        if (!nodes_to_rebase.empty()) {
            auto& pivot_node = nodes_to_rebase[lowest_node_found_index];
            SymbolLink * link_for_base;
            auto& canonical_node = *(pivot_node.linkFromCanonicalNode->origin);


            for (auto& node_to_move : nodes_to_rebase) {
                if (node_to_move.is_pivot) {
                    // Link to base can be undone
                    node_to_move.linkToMove->unlink_and_reset();


                    link_for_base = node_to_move.linkToMove;
                    link_for_base->link_type = compose(node_to_move.linkFromCanonicalNode->link_type,
                        node_to_move.relationToBase);


                } else {




                }
                // Either way, node should be removed from base
//                node_to_move.linkToMove.unlink();

            }
        }
    }


    namespace {
        struct NodeAndIter {
            SymbolTree::SymbolNode * node;
            SymbolTree::SymbolLinkIterator iter;
            EqualityType relationToBase = EqualityType::none;

            constexpr explicit NodeAndIter(SymbolTree::SymbolNode * the_node, EqualityType rtb) noexcept
                    : node(the_node), iter(the_node->begin()), relationToBase(rtb) { }

        };
    }

    size_t SymbolTree::SymbolNode::find_already_linked(std::vector<RebaseInfoImpl>& rebase_list) {
        rebase_list.clear();

        size_t lowest_node_found_index = -1;
        symbol_name_t lowest_node_found = std::numeric_limits<symbol_name_t>::max();

        // Scan children; kinda recursively
        SymbolNode * node_cursor = this;
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
                    entry.relationToPivot = EqualityType::equal; // We are the pivot node.
                } else {
                    entry.is_pivot = false;
                    entry.relationToPivot = compose(pivot_entry.relationToBase, entry.relationToBase);
                }
            }
        }

        return lowest_node_found_index;
    }


}
