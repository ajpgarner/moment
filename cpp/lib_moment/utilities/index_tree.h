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

    template<std::integral look_up_t, typename value_t = std::size_t>
    class IndexTree {
    public:
        using ValueType = value_t;

    private:
        look_up_t id;
        std::optional<value_t> _value;
        std::vector<std::unique_ptr<IndexTree<look_up_t, value_t>>> children;

    public:
        template<bool is_const>
        class Iterator {
        public:
            friend class Iterator<!is_const>;

            using IndexTreeT = typename std::conditional<is_const, const IndexTree<look_up_t, value_t>,
                                                                     IndexTree<look_up_t, value_t>>::type;
            using iterator_category = std::input_iterator_tag;
            using difference_type = typename std::make_signed<look_up_t>::type;
            using value_type = IndexTreeT;

            struct end_flag_t{};
        private:
            struct recursion_stack {
            public:
                IndexTreeT * node = nullptr;
                look_up_t next_child = 0;

                recursion_stack() = default;

                recursion_stack(const recursion_stack&) = default;

                recursion_stack(IndexTreeT * node, look_up_t next_child)
                    : node{node}, next_child{next_child} { }

                [[nodiscard]] constexpr bool has_next_child() const noexcept {
                    return next_child < node->children.size();
                }

                [[nodiscard]] bool operator==(const typename Iterator<false>::recursion_stack& rhs) const noexcept {
                    return (this->node == rhs.node) && (this->next_child == rhs.next_child);
                }

                [[nodiscard]] bool operator==(const typename Iterator<true>::recursion_stack& rhs) const noexcept {
                    return (this->node == rhs.node) && (this->next_child == rhs.next_child);
                }

            };
            std::vector<recursion_stack> stack;

        public:
            /** Begin state of iterator - stack points at base element, with option to descend to its children. */
            explicit constexpr Iterator(IndexTreeT& base) {
                this->stack.emplace_back(&base, 0);
            }

            /** End state of iterator - empty stack */
            explicit constexpr Iterator() = default;

            /** How deep in the stack are we?
             * Undefined if iterator is in end state. */
            [[nodiscard]] size_t current_depth() {
                return this->stack.size() - 1;
            }

            /** Is iterator in end state? */
            [[nodiscard]] bool operator!() const noexcept {
                return this->stack.empty();
            }

            /** Is iterator not in end state? */
            [[nodiscard]] explicit operator bool() const noexcept {
                return !this->stack.empty();
            }

            /** Compare iterators */
            template<bool other_iter_const>
            [[nodiscard]] bool operator== (const Iterator<other_iter_const>& other) const noexcept {
                return std::equal(this->stack.begin(), this->stack.end(),
                                  other.stack.begin(), other.stack.end());
            }

            /** Compare iterators */
            template<bool other_iter_const>
            [[nodiscard]] bool operator!=(const Iterator<other_iter_const>& other) const noexcept {
                return !std::equal(this->stack.begin(), this->stack.end(),
                                   other.stack.begin(), other.stack.end());
            }

            /**
             * Makes an index to the node in the tree.
             * Undefined if iterator is in end state. */
            [[nodiscard]] std::vector<look_up_t> lookup_index() const {
                assert(!this->stack.empty());
                std::vector<look_up_t> output;
                output.reserve(this->stack.size() - 1);
                std::transform(this->stack.begin(), this->stack.end() - 1, std::back_inserter(output),
                               [](const recursion_stack& stack_elem) {
                                    return stack_elem.node->children[stack_elem.next_child - 1]->id;
                               });
                return output;
            }

            [[nodiscard]] IndexTreeT& operator*() const noexcept {
                assert(!this->stack.empty());
                return *(this->stack.back().node);
            }

            [[nodiscard]] IndexTreeT* operator->() const noexcept {
                assert(!this->stack.empty());
                return this->stack.back().node;
            }

            Iterator& operator++() {
                // Can we trivially descend?
                if (this->try_descend()) {
                    return *this;
                }

                // No more children, so we have to unwind the stack until we have children, or are done
                do {
                    this->stack.pop_back();

                    // Iterator is in end state?
                    if (this->stack.empty()) {
                        return *this;
                    }

                } while (!this->stack.back().has_next_child());

                // Unwound, but still have children, so descend again:
                [[maybe_unused]] const bool successful_descend = this->try_descend();
                assert(successful_descend);
                return *this;
            }

            [[deprecated("This makes an expensive copy. Prefer ++iter;")]]
            [[nodiscard]] const Iterator operator++(int) {
                Iterator useless_copy{*this};
                ++(*this);
                return useless_copy;
            }

        private:
            inline bool try_descend() {
                assert(!this->stack.empty());
                auto& last_in_stack = this->stack.back();
                if (last_in_stack.has_next_child()) {
                    const auto descend_child = last_in_stack.next_child;
                    // Inc.
                    ++last_in_stack.next_child;

                    // Descend
                    this->stack.emplace_back(last_in_stack.node->children[descend_child].get(), 0);
                    return true;
                }
                return false;
            }
        };

        static_assert(std::input_iterator<Iterator<true>>);
        static_assert(std::input_iterator<Iterator<false>>);

    public:
        IndexTree() : id{std::numeric_limits<look_up_t>::max()} { }

        explicit IndexTree(look_up_t id) : id{id} { }

        IndexTree(look_up_t id, value_t value) : id{id}, _value{value} { }

        IndexTree(const IndexTree& rhs) = delete;

        IndexTree(IndexTree&& rhs) = default;

        std::optional<value_t> value() const noexcept { return this->_value; }

        /**
         * Add an entry to the tree
         * @param look_up_str Span over index sequence
         * @param entry_value The entry to write to the tree
         */
        void add(std::span<const look_up_t> look_up_str, value_t entry_value) {
            // If we have fully descended, write index
            if (look_up_str.empty()) {
                this->_value = std::move(entry_value);
                return;
            }
            const auto& current_index = look_up_str.front();

            // Create (or find) node
            auto* next_node = this->add_node(current_index);

            // Recursively descend
            next_node->add(look_up_str.subspan(1), std::move(entry_value));
        }

        /**
         * Add an entry to the tree if it doesn't already exist
         * @param look_up_str Span over index sequence
         * @param entry_index The entry to write to the tree
         */
         std::pair<value_t, bool>
         add_if_new(std::span<const look_up_t> look_up_str, value_t entry_value, bool did_addition = false) {
            // If we have fully descended, write index
            if (look_up_str.empty()) {
                if (did_addition) {
                    assert(!this->_value.has_value());
                    this->_value = std::move(entry_value);
                    return {this->_value.value(), true};
                } else {
                    if (!this->_value.has_value()) {
                        this->_value = std::move(entry_value);
                        return {this->_value.value(), true};
                    } else {
                        return {this->_value.value(), false};
                    }
                }
            }

            // Create (or find) next node
            const auto& current_index = look_up_str.front();
            auto* next_node = this->add_node(current_index, &did_addition);

            // Recursively descend
            return next_node->add_if_new(look_up_str.subspan(1), std::move(entry_value), did_addition);
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
        std::optional<value_t> find(std::span<const look_up_t> look_up_str) const noexcept {
            if (look_up_str.empty()) {
                return this->_value;
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

        /** Begin read-write iterator */
        [[nodiscard]] Iterator<false> begin() {
            return Iterator<false>(*this);
        }

        /** End read-write iterator */
        [[nodiscard]] Iterator<false> end() {
            return Iterator<false>();
        }

        /** Begin read-only iterator */
        [[nodiscard]] Iterator<true> begin() const {
            return Iterator<true>(*this);
        }

        /** End read-only iterator */
        [[nodiscard]] Iterator<true> end() const {
            return Iterator<true>();
        }

        /** Begin read-only iterator */
        [[nodiscard]] Iterator<true> cbegin() const {
            return Iterator<true>(*this);
        }

        /** End read-only iterator */
        [[nodiscard]] Iterator<true> cend() const {
            return Iterator<true>();
        }
    };

}