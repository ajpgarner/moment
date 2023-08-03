/**
 * index_tree.h
 * 
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include <cassert>

#include <algorithm>
#include <concepts>
#include <memory>
#include <optional>
#include <span>
#include <vector>

namespace Moment {

    template<std::integral look_up_t, std::integral index_t = std::size_t>
    class IndexTree {
    private:
        look_up_t id;
        std::optional<index_t> _index;
        std::vector<std::unique_ptr<IndexTree<look_up_t, index_t>>> children;

    public:
        IndexTree() : id{std::numeric_limits<look_up_t>::max()} { }

        explicit IndexTree(look_up_t id) : id{id} { }

        IndexTree(look_up_t id, index_t index) : id{id}, _index{index} { }

        IndexTree(const IndexTree& rhs) = delete;

        IndexTree(IndexTree&& rhs) = default;

        std::optional<index_t> index() const noexcept { return this->_index; }

        /**
         * Add an entry to the tree
         * @param look_up_str Span over index sequence
         * @param entry_index The entry to write to the tree
         */
        void add(std::span<const look_up_t> look_up_str, index_t entry_index) {
            // If we have fully descended, write index
            if (look_up_str.empty()) {
                this->_index = entry_index;
                return;
            }
            const auto& current_index = look_up_str.front();

            // Create (or find) node
            auto* next_node = this->add_node(current_index);

            // Recursively descend
            next_node->add(look_up_str.subspan(1), entry_index);
        }

        /**
         * Add an entry to the tree if it doesn't already exist
         * @param look_up_str Span over index sequence
         * @param entry_index The entry to write to the tree
         */
         std::pair<index_t, bool>
         add_if_new(std::span<const look_up_t> look_up_str, index_t entry_index, bool did_addition = false) {
            // If we have fully descended, write index
            if (look_up_str.empty()) {
                if (did_addition) {
                    assert(!this->_index.has_value());
                    this->_index = entry_index;
                    return {entry_index, true};
                } else {
                    if (!this->_index.has_value()) {
                        this->_index = entry_index;
                        return {entry_index, true};
                    } else {
                        return {this->_index.value(), false};
                    }
                }
            }

            // Create (or find) next node
            const auto& current_index = look_up_str.front();
            auto* next_node = this->add_node(current_index, &did_addition);

            // Recursively descend
            return next_node->add_if_new(look_up_str.subspan(1), entry_index, did_addition);
        }

        /**
         * Add an empty node to the tree
         * @param current_index The index in the tree to insert.
         * @param addition_flag Output param, writes true if node does not exist
         * @return Pointer to (possibly newly added) node.
         */
         IndexTree * add_node(look_up_t current_index, bool * addition_flag = nullptr) {
            // Find new node, (or node just before)
            auto iter_to_child = std::lower_bound(this->children.begin(), this->children.end(),
                                                     current_index,
                                                     [](const auto& x, auto y) {
                                                        return x->id < y;
                                                     });

            // If not perfectly matching, add new node in situ
            if ((iter_to_child == this->children.end()) || ((*iter_to_child)->id != current_index)) {
                iter_to_child = this->children.emplace(iter_to_child, std::make_unique<IndexTree>(current_index));
                if (addition_flag != nullptr) {
                    *addition_flag = true;
                }
            }

            // Return pointer to (possibly) added node.
            return (*iter_to_child).get();
        }

        /**
         * Attempt to read an entry from the tree
         */
        std::optional<index_t> find(std::span<const look_up_t> look_up_str) const noexcept {
            if (look_up_str.empty()) {
                return this->_index;
            }
            const auto& current_index = look_up_str.front();

            // Find new node, (or node just before)
            auto iter_to_child = std::lower_bound(this->children.begin(), this->children.end(),
                                                  current_index,
                                                  [](const auto& x, auto y) {
                                                      return x->id < y;
                                                  });
            if ((iter_to_child == this->children.end()) || ((*iter_to_child)->id != current_index)) {
                return std::nullopt;
            }

            return (*iter_to_child)->find(look_up_str.subspan(1));
        }

        /**
         * Finds with hints
         */
        std::pair<const IndexTree*, std::span<const look_up_t>>
        find_node_or_return_hint(std::span<const look_up_t> look_up_str) const noexcept {
            // Full match
            if (look_up_str.empty()) {
                return {this, std::span<const look_up_t>{}};
            }

            // Find new node, (or node just before)
            const auto& current_index = look_up_str.front();
            auto iter_to_child = std::lower_bound(this->children.begin(), this->children.end(),
                                                  current_index,
                                                  [](const auto& x, auto y) {
                                                      return x->id < y;
                                                  });
            if ((iter_to_child == this->children.end()) || ((*iter_to_child)->id != current_index)) {
                // No match, return 'rebased' tree.
                return {this, look_up_str};
            }

            // Found, so descend
            return (*iter_to_child)->find_node_or_return_hint(look_up_str.subspan(1));
        }

        /**
         * Attempt to find a node in the index tree
         */
        const IndexTree * find_node(std::span<const look_up_t> look_up_str) const noexcept {
            if (look_up_str.empty()) {
                return this;
            }
            const auto& current_index = look_up_str.front();

            const auto * child = this->find_node(current_index);
            if (child == nullptr) {
                return nullptr;
            }
            return child->find_node(look_up_str.subspan(1));
        }


        /**
         * Attempt to find a node in the index tree
         */
        const IndexTree * find_node(const look_up_t current_index) const noexcept {
            // Find new node, (or node just before)
            auto iter_to_child = std::lower_bound(this->children.begin(), this->children.end(),
                                                  current_index,
                                                  [](const auto& x, auto y) {
                                                      return x->id < y;
                                                  });
            if ((iter_to_child == this->children.end()) || ((*iter_to_child)->id != current_index)) {
                return nullptr;
            }
            return (*iter_to_child).get();
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