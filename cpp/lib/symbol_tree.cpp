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

            this->tree_links.emplace_back(source_node, target_node, link_type);
            LinkedSymbolLink * this_link = &this->tree_links[insert_index];
            source_node->link_back(this_link);

            ++insert_index;
        }
    }

    void SymbolTree::SymbolNode::link_back(SymbolTree::LinkedSymbolLink* link) noexcept {
        if (this->last_link != nullptr) {
            this->last_link->next = link;
            link->prev = this->last_link;
            link->next = nullptr;
            this->last_link = link;
        } else {
            this->first_link = link;
            this->last_link = link;
        }
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

    namespace {
        struct NodeAndIter {
            SymbolTree::SymbolNode * node;
            SymbolTree::LinkedSymbolLinkIterator iter;
            EqualityType relationToBase = EqualityType::none;

            constexpr explicit NodeAndIter(SymbolTree::SymbolNode * the_node) noexcept
                : node(the_node), iter(the_node->begin()) { }

        };
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

        // Scan children; kinda recursively
        SymbolNode * node_cursor = this;
        std::stack<NodeAndIter> recurse_stack;
        recurse_stack.emplace(node_cursor);

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
            auto& current_link = *(stack_frame.iter);

            // Node's child has canonical origin, no need to traverse deeper...!
            if (current_link.target->canonical_origin != nullptr) {
                // TODO: Register CO

                // Advance current iterator to next child, and continue
                ++(stack_frame.iter);
                continue;
            }

            // Node's child has children
            if (!current_link.target->empty()) {
                // Check not recursive
                if (current_link.target != current_link.origin) {

                    // Go down one level in the stack
                    recurse_stack.emplace(current_link.target);
                    continue;
                }
            }

            // Otherwise, advance current iterator
            ++(stack_frame.iter);
        }

    }
}
