/**
 * symbol_tree.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "symbol_tree.h"

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


    void SymbolTree::release_link(SymbolTree::SymbolLink * link) {
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

        // Go through nodes, executing algorithm
        const size_t symbol_count = this->tree_nodes.size();
        for (symbol_name_t node_id = 0; node_id < symbol_count; ++node_id) {
            this->tree_nodes[node_id].simplify();
        }

        // Propagate real & imaginary nullity
        propagate_nullity();

        // Nodes that are formally zero should alias to zero
        sweep_zero();

        // Count aliases
        count_noncanonical_nodes();

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


    size_t SymbolTree::count_noncanonical_nodes() {
        this->num_aliases = 0;
        for (auto& node : this->tree_nodes) {
            if (!node.unaliased()) {
                ++this->num_aliases;
            }
        }
        return this->num_aliases;
    }

    void SymbolTree::sweep_zero() {
        const size_t symbol_count = this->count_nodes();

        auto &zeroNode = this->tree_nodes[0];
        for (symbol_name_t node_id = 1; node_id < symbol_count; ++node_id) {
            auto &theNode = this->tree_nodes[node_id];

            // Early exit on nodes that have a parent
            if (!theNode.unaliased()) {
                continue;
            }

            if (theNode.is_zero()) {
                auto * newLink = this->getAvailableLink();
                assert(newLink != nullptr);
                newLink->link_type = EqualityType::equal;
                newLink->target = &theNode;
                zeroNode.subsume(newLink);
            }
        }
    }

    void SymbolTree::propagate_nullity() {
        for (auto& node : this->tree_nodes) {
            if (!node.unaliased()) {
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

    std::unique_ptr<SymbolSet> SymbolTree::export_symbol_set() const {
        return std::make_unique<SymbolSet>(*this);
    }


}
