/**
 * symbol_tree.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once
#include "symbol_set.h"
#include <iosfwd>
#include <iterator>
#include <concepts>

namespace NPATK {

    class SymbolTree {
    public:

        struct SymbolNode;

        struct SymbolLink {
        public:
            EqualityType link_type;
            SymbolNode * origin = nullptr;
            SymbolNode * target = nullptr;
        public:
             SymbolLink(SymbolNode * from, SymbolNode * to, EqualityType link) noexcept
                : origin(from), target(to), link_type(link) { }
        };

        struct LinkedSymbolLink : public SymbolLink {
            LinkedSymbolLink * prev = nullptr;
            LinkedSymbolLink * next = nullptr;
        public:
            LinkedSymbolLink(SymbolNode * from, SymbolNode * to, EqualityType link) noexcept
                : SymbolLink(from, to, link) { }
        };

        template<bool is_const>
        class LinkedSymbolIteratorBase {
        public:
            using value_type = std::conditional_t<is_const, LinkedSymbolLink, LinkedSymbolLink const>;
            using ptr_type = std::conditional_t<is_const, LinkedSymbolLink*, LinkedSymbolLink const*>;

        private:
            ptr_type cursor = nullptr;

        public:
            LinkedSymbolIteratorBase() noexcept = default;

            explicit LinkedSymbolIteratorBase(ptr_type lsl) noexcept : cursor(lsl) { }

            constexpr bool operator== (const LinkedSymbolIteratorBase& rhs) const noexcept {
                return this->cursor == rhs.cursor;
            }

            constexpr bool operator!= (const LinkedSymbolIteratorBase& rhs) const noexcept {
                return this->cursor != rhs.cursor;
            }

            constexpr value_type& operator*() noexcept {
                return *(this->cursor);
            }

            constexpr const value_type& operator*() const noexcept {
                return *(this->cursor);
            }

            constexpr LinkedSymbolIteratorBase<is_const>& operator++() noexcept {
                this->cursor = this->cursor->next;
                return *this;
            }

            constexpr LinkedSymbolIteratorBase<is_const>& operator--() noexcept {
                this->cursor = this->cursor->prev;
                return *this;
            }
        };

        using LinkedSymbolLinkIterator = LinkedSymbolIteratorBase<false>;
        using LinkedSymbolLinkConstIterator = LinkedSymbolIteratorBase<true>;


        struct SymbolNode {
        public:
            /** Name of the symbol */
            symbol_name_t id;

            /** Canonical link to symbol with lower ID, if any*/
            SymbolLink * canonical_origin = nullptr;

            /** True if constraints dictate real part must be zero */
            bool real_is_zero = false;

            /** True if constraints dictate imaginary part must be zero */
            bool im_is_zero = false;

        private:
            /** First link, if any, to symbols with higher ID */
            LinkedSymbolLink * first_link = nullptr;

            /** Final link, if any, to symbols with higher ID */
            LinkedSymbolLink * last_link = nullptr;

        public:
            explicit SymbolNode(symbol_name_t name) noexcept : id(name) { }

            [[nodiscard]] LinkedSymbolLinkIterator begin() noexcept {
                return LinkedSymbolLinkIterator{this->first_link};
            }

            [[nodiscard]] LinkedSymbolLinkIterator end() noexcept {
                return LinkedSymbolLinkIterator{};
            }

            [[nodiscard]] LinkedSymbolLinkConstIterator begin() const noexcept {
                return this->cbegin();
            }

            [[nodiscard]] LinkedSymbolLinkConstIterator end() const noexcept {
                return this->cend();
            }

            [[nodiscard]] LinkedSymbolLinkConstIterator cbegin() const noexcept {
                return LinkedSymbolLinkConstIterator{this->first_link};
            }

            [[nodiscard]] LinkedSymbolLinkConstIterator cend() const noexcept {
                return LinkedSymbolLinkConstIterator{};
            }

            void link_back(LinkedSymbolLink* link) noexcept;

            [[nodiscard]] bool is_zero() const { return real_is_zero && im_is_zero; }

            void relink();

            size_t find_canonical_origins(SymbolLink*& lowest_origin) noexcept;
        };

    private:
        std::vector<SymbolNode> tree_nodes;
        std::vector<LinkedSymbolLink> tree_links;

    public:
        explicit SymbolTree(const SymbolSet& symbols);

        [[nodiscard]] size_t count_nodes() const noexcept { return tree_nodes.size(); }

        [[nodiscard]] size_t max_links() const noexcept { return tree_links.size(); }

        void simplify();

        const SymbolNode& operator[](size_t index) {
            return this->tree_nodes[index];
        }

        friend std::ostream& operator<<(std::ostream& os, const SymbolTree& st);

    };
}