/**
 * symbol_tree_symbol_link.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */

#include "symbol_tree.h"

#include <cassert>

namespace NPATK {
    std::pair<SymbolTree::SymbolLink *, SymbolTree::SymbolLink *> SymbolTree::SymbolLink::detach() noexcept {
        assert(this->prev != this);
        assert(this->next != this);

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


    std::pair<bool, bool> SymbolTree::SymbolLink::merge_in(EqualityType extra_link) noexcept {
        this->link_type |= extra_link;
        auto [re_is_zero, im_is_zero] = this->implies_zero();
        if (this->origin != nullptr) {
            this->origin->real_is_zero |= re_is_zero;
            this->origin->im_is_zero |= im_is_zero;
        }
        if (this->target != nullptr) {
            this->target->real_is_zero |= re_is_zero;
            this->target->im_is_zero |= im_is_zero;
        }
        return {re_is_zero, im_is_zero};
    }
}