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


    //struct SymbolNode;
    //struct SymbolLink;
    //struct LinkedSymbolLink;


    class SymbolTree {
    public:

        struct SymbolNode;

        template<bool is_const>
        class SymbolLinkIteratorBase;

        struct SymbolLink {
        public:
            EqualityType link_type = EqualityType::none;
            SymbolNode * origin = nullptr;
            SymbolNode * target = nullptr;

        private:
            SymbolLink * prev = nullptr;
            SymbolLink * next = nullptr;

        public:

             SymbolLink(SymbolNode * to, EqualityType link) noexcept
                : target(to), link_type(link) { }

            /**
             * Detach link from origin, and set all values to 0.
             * @return Pair, with values of prev and next prior to delink
             */
            std::pair<SymbolLink*, SymbolLink*> unlink_and_reset() noexcept;

            template<bool is_const>
            friend class SymbolLinkIteratorBase;

            friend struct SymbolNode;

        };

        template<bool is_const>
        class SymbolLinkIteratorBase {

        public:
            using value_type = std::conditional_t<is_const, SymbolLink const, SymbolLink>;
            using ptr_type = std::conditional_t<is_const, SymbolLink const*, SymbolLink*>;
            using ref_type = std::conditional_t<is_const,  const SymbolLink &, SymbolLink&>;

        private:
            ptr_type cursor = nullptr;

        public:
            SymbolLinkIteratorBase() noexcept = default;

            explicit SymbolLinkIteratorBase(ptr_type lsl) noexcept : cursor(lsl) { }

            constexpr bool operator== (const SymbolLinkIteratorBase& rhs) const noexcept {
                return this->cursor == rhs.cursor;
            }

            constexpr bool operator!= (const SymbolLinkIteratorBase& rhs) const noexcept {
                return this->cursor != rhs.cursor;
            }

            constexpr ref_type operator*() noexcept {
                return *(this->cursor);
            }

            constexpr SymbolLinkIteratorBase<is_const>& operator++() noexcept {
                this->cursor = this->cursor->next;
                return *this;
            }

            constexpr SymbolLinkIteratorBase<is_const>& operator--() noexcept {
                this->cursor = this->cursor->prev;
                return *this;
            }
        };

        using SymbolLinkIterator = SymbolLinkIteratorBase<false>;
        using SymbolLinkConstIterator = SymbolLinkIteratorBase<true>;

        struct SymbolNode {
            friend struct SymbolLink;

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
            SymbolLink * first_link = nullptr;

            /** Final link, if any, to symbols with higher ID */
            SymbolLink * last_link = nullptr;

        public:
            explicit SymbolNode(symbol_name_t name) noexcept : id(name) { }

            [[nodiscard]] SymbolLinkIterator begin() noexcept {
                return SymbolLinkIterator{this->first_link};
            }

            [[nodiscard]] SymbolLinkIterator end() noexcept {
                return SymbolLinkIterator{};
            }

            [[nodiscard]] SymbolLinkConstIterator begin() const noexcept {
                return this->cbegin();
            }

            [[nodiscard]] SymbolLinkConstIterator end() const noexcept {
                return this->cend();
            }

            [[nodiscard]] SymbolLinkConstIterator cbegin() const noexcept {
                return SymbolLinkConstIterator{this->first_link};
            }

            [[nodiscard]] SymbolLinkConstIterator cend() const noexcept {
                return SymbolLinkConstIterator{};
            }

            [[nodiscard]] constexpr bool is_zero() const {
                return real_is_zero && im_is_zero;
            }

            [[nodiscard]] constexpr bool empty() const noexcept {
                return this->first_link == nullptr;
            }

            void link_back(SymbolLink* link) noexcept;

            void relink();

        private:
            // XXX: Move to impl.
            struct RebaseInfoImpl {
                SymbolTree::SymbolLink * linkToMove;
                SymbolTree::SymbolLink * linkFromCanonicalNode;

                EqualityType relationToBase;
                EqualityType relationToPivot = EqualityType::none;
                bool is_pivot = false;

                RebaseInfoImpl(SymbolTree::SymbolLink * it_link,
                         SymbolTree::SymbolLink * can_link,
                         EqualityType rtb) noexcept :
                            linkToMove(it_link),
                            linkFromCanonicalNode(can_link),
                            relationToBase(rtb) { }
            };

            // XXX: Move to impl.
            size_t find_already_linked(std::vector<RebaseInfoImpl>& rebase_list);



        };

    private:
        std::vector<SymbolNode> tree_nodes;
        std::vector<SymbolLink> tree_links;

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