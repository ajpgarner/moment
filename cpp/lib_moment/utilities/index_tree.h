/**
 * index_tree.h
 * 
 * Copyright (c) 2023 Austrian Academy of Sciences
 */
#pragma once

#include <cassert>

#include <concepts>
#include <optional>
#include <span>
#include <vector>

namespace Moment {

    template<std::integral look_up_t, std::integral index_t = std::size_t>
    class IndexTree {
    private:
        look_up_t id;
        std::optional<index_t> index;
        std::vector<IndexTree> children;

    public:
        IndexTree() : id{std::numeric_limits<look_up_t>::max()} { }

        explicit IndexTree(look_up_t id) : id{id} { }

        IndexTree(look_up_t id, index_t index) : id{id}, index{index} { }

        /**
         * Add an entry to the tree
         * @param look_up_str Span over index sequence
         * @param entry_index The entry to write to the tree
         */
        void add(std::span<const look_up_t> look_up_str, index_t entry_index) {
            // If we have fully descended, write index
            if (look_up_str.empty()) {
                this->index = entry_index;
                return;
            }
            const auto& current_index = look_up_str.front();

            // Find new node, (or node just before)
            auto iter_to_child = std::lower_bound(this->children.begin(), this->children.end(),
                                                     current_index,
                                                     [](const auto& x, auto y) {
                                                        return x.id < y;
                                                     });

            // If not perfectly matching, add new node
            if ((iter_to_child == this->children.end()) || (iter_to_child->id != current_index)) {
                iter_to_child = this->children.emplace(iter_to_child, current_index);
            }

            // Recursively descend
            iter_to_child->add(look_up_str.subspan(1), entry_index);
        }

        /**
         * Attempt to read an entry from the tree
         */
        std::optional<index_t> find(std::span<const look_up_t> look_up_str) const noexcept {
            if (look_up_str.empty()) {
                return this->index;
            }
            const auto& current_index = look_up_str.front();

            // Find new node, (or node just before)
            auto iter_to_child = std::lower_bound(this->children.begin(), this->children.end(),
                                                  current_index,
                                                  [](const auto& x, auto y) {
                                                      return x.id < y;
                                                  });
            if ((iter_to_child == this->children.end()) || (iter_to_child->id != current_index)) {
                return std::nullopt;
            }

            return iter_to_child->find(look_up_str.subspan(1));
        }

        /**
         * True, if tree node has no children
         * @return
         */
        [[nodiscard]] bool leaf() const noexcept {
            return this->children.empty();
        }
    };

}