/**
 * symbol_tree.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_tree.h"
#include "symbol_tree_simplify_impl.h"

#include <algorithm>
#include <iostream>

namespace NPATK {
    std::ostream& operator<<(std::ostream& os, const SymbolTree& st) {
        for (auto symbol : st.tree_nodes) {
            os << symbol.id;
            if (symbol.is_zero()) {
                os << " [0]";
            } else if (symbol.im_is_zero) {
                os << " [re]";
            } else if (symbol.real_is_zero) {
                os << " [im]";
            }

            bool once = false;

            for (const auto& lsl : symbol) {
                 if (once) {
                     os << ",\t";
                 } else {
                     os << "\t<-\t";
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


    SymbolTree::SymbolTree(const SymbolSet &symbols)
        : packing_map{symbols.packing_key}, unpacking_map{symbols.unpacking_key}
    {
        assert(symbols.is_packed());
        this->make_nodes_and_links(symbols);
    }


    SymbolTree::SymbolTree(SymbolSet&& symbols) {
        assert(symbols.is_packed());
        this->make_nodes_and_links(symbols);
        this->packing_map.swap(symbols.packing_key);
        this->unpacking_map.swap(symbols.unpacking_key);
    }

    SymbolTree::SymbolTree() = default;

    void SymbolTree::make_nodes_and_links(const SymbolSet &symbols) {
        /** Create nodes */
        size_t symbol_count = symbols.symbol_count();
        this->tree_nodes.reserve(symbol_count);


        for (const auto& symbol : symbols.Symbols) {
            Symbol unpacked{symbol.second};

            // Replace symbol ID with its unpacked variant
            auto [found, upk] = symbols.unpacked_key(symbol.first);
            assert(found);
            unpacked.id = upk;

            this->tree_nodes.emplace_back(*this, unpacked);
        }

        /** Create links */
        this->tree_links.reserve(symbols.link_count());
        for (const auto [key, link_type] : symbols.Links) {
            SymbolNode * source_node = &this->tree_nodes[key.first];
            SymbolNode * target_node = &this->tree_nodes[key.second];
            this->tree_links.emplace_back(*this, target_node, link_type);
            SymbolLink * new_link = &(this->tree_links[this->tree_links.size()-1]);
            source_node->insert_ordered(new_link);
        }
    }

    namespace {
        /** False, if link is already in list of released links */
        [[nodiscard]] inline bool debug_assert_good_release(const std::vector<SymbolTree::SymbolLink*>& links,
                                       const SymbolTree::SymbolLink * toRelease) {
            return (toRelease != nullptr) && !std::any_of(links.cbegin(), links.cend(),
                                                         [=](SymbolTree::SymbolLink* lhs) { return lhs == toRelease;} );
        }
    }


    void SymbolTree::releaseLink(SymbolTree::SymbolLink * link) {
        assert(debug_assert_good_release(this->available_links, link));
        this->available_links.push_back(link);
    }

    SymbolTree::SymbolLink * SymbolTree::getAvailableLink() {
        if (!this->available_links.empty()) {
            auto * link = this->available_links.back();
            this->available_links.pop_back();
            return link;
        }
        return nullptr;
    }


    void SymbolTree::simplify() {
        if (this->done_simplification) {
            return;
        }

        detail::SymbolNodeSimplifyImpl impl{*this};
        impl.simplify();

        this->done_simplification = true;
    }

    SymbolExpression SymbolTree::substitute(SymbolExpression expr) const noexcept {
        // First, look up name of symbol as keyed
        auto key_iter = this->packing_map.find(expr.id);
        if (key_iter == this->packing_map.end()) {
            // Did not find expr, cannot simplify
            return expr;
        }

        // Now find associated node in tree...
        assert((key_iter->second >= 0) && (key_iter->second < this->tree_nodes.size()));
        const auto& node = this->tree_nodes[key_iter->second];
        auto canon_expr = node.canonical_expression();

        // Return 0 if node must be zero.
        if (node.is_zero()) {
            return SymbolExpression{0};
        }

        canon_expr.conjugated = (canon_expr.conjugated != expr.conjugated);
        canon_expr.negated = (canon_expr.negated != expr.negated);

        // Pure imaginary node..., convert * to -
        if (node.real_is_zero && canon_expr.conjugated) {
                canon_expr.conjugated = false;
                canon_expr.negated = !canon_expr.negated;
        }
        // Pure real node
        if (node.im_is_zero) {
            canon_expr.conjugated = false;
        }

        return canon_expr;
    }

    std::unique_ptr<SymbolSet> SymbolTree::export_symbol_set() const {
        return std::make_unique<SymbolSet>(*this);
    }


}
