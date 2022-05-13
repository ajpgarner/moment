/**
 * symbol_tree.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_tree.h"
#include "symbol_tree_simplify_impl.h"

#include <iostream>
#include <stack>

namespace NPATK {
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


    std::pair<SymbolTree::SymbolLink *, SymbolTree::SymbolLink *> SymbolTree::SymbolLink::detach() noexcept {
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

        // Detach:
        this->origin = nullptr;
        this->prev = nullptr;
        this->next = nullptr;
        return old_values;
    }


    std::pair<SymbolTree::SymbolLink *, SymbolTree::SymbolLink *>
            SymbolTree::SymbolLink::detach_and_reset() noexcept {
        auto old_values = this->detach();

        // Also reset target info:
        this->target = nullptr;
        this->link_type = EqualityType::none;
        return old_values;
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
            } else if (link->target->id == hint->target->id) {
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
            linkToMove.target->canonical_origin = &linkToMove;

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
        detail::SymbolNodeSimplifyImpl simpImpl{*this};
        simpImpl.simplify();
    }


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
            source_node->insert_back(this_link);

            ++insert_index;
        }
    }


    void SymbolTree::simplify() {
        const size_t symbol_count = this->tree_nodes.size();
        for (symbol_name_t base_id = 0; base_id < symbol_count; ++base_id) {
            this->tree_nodes[base_id].simplify();
        }
    }


}
