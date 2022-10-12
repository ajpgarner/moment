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
#include <memory>

namespace NPATK {
    namespace detail {
        /**
         * Class splitting out some of the messy methods required to simplify.
         */
        class SymbolNodeSimplifyImpl;
    }

    class SymbolTree {
    public:
        struct SymbolNode;

        template<bool is_const>
        class SymbolLinkIteratorBase;

        struct SymbolLink {
        private:
            /** Reference to owning tree */
            SymbolTree& the_tree;

        public:
            EqualityType link_type = EqualityType::none;
            SymbolNode * origin = nullptr;
            SymbolNode * target = nullptr;

        private:
            SymbolLink * prev = nullptr;
            SymbolLink * next = nullptr;

        public:

             SymbolLink(SymbolTree& tree, SymbolNode * to, EqualityType link) noexcept
                : the_tree{tree}, target(to), link_type(link) { }

            /**
             * Detach link from origin, but keep target and equality type info.
             * @return Pair, with values of prev and next prior to delink
             */
            std::pair<SymbolLink*, SymbolLink*> detach() noexcept;

            /**
             * Detach link from origin, and reset all values.
             * @return Pair, with values of prev and next prior to delink
             */
            std::pair<SymbolLink*, SymbolLink*> detach_and_reset() noexcept;


            /**
             * Tests if the link type implies that the real or imaginary parts of associated symbols must be zero.
             * @return Pair, first: true if real part is zero; second: true if imaginary part is zero.
             */
            [[nodiscard]] constexpr std::pair<bool, bool> implies_zero() const noexcept {
                if ((origin == target) && (target != nullptr)) {
                    return NPATK::reflexive_implies_zero(this->link_type);
                }
                return NPATK::implies_zero(this->link_type);
            };


            template<bool is_const>
            friend class SymbolLinkIteratorBase;

            friend struct SymbolNode;
            friend class detail::SymbolNodeSimplifyImpl;
        };

        template<bool is_const>
        class SymbolLinkIteratorBase {

        public:
            using value_type = std::conditional_t<is_const, SymbolLink const, SymbolLink>;
            using ptr_type = std::conditional_t<is_const, SymbolLink const*, SymbolLink*>;
            using ref_type = std::conditional_t<is_const,  const SymbolLink &, SymbolLink&>;
            using cptr_type = SymbolLink const*;

        private:
            ptr_type cursor = nullptr;

        public:
            constexpr SymbolLinkIteratorBase() noexcept = default;

            constexpr explicit SymbolLinkIteratorBase(ptr_type lsl) noexcept : cursor(lsl) { }

            constexpr bool operator== (const SymbolLinkIteratorBase& rhs) const noexcept {
                return this->cursor == rhs.cursor;
            }

            constexpr bool operator!= (const SymbolLinkIteratorBase& rhs) const noexcept {
                return this->cursor != rhs.cursor;
            }

            [[nodiscard]] constexpr ref_type operator*() noexcept {
                return *(this->cursor);
            }

            [[nodiscard]] constexpr ptr_type operator->() noexcept {
                return this->cursor;
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

        struct SymbolNode : public Symbol {
            friend struct SymbolLink;

        private:
            /** Reference to owning tree */
            SymbolTree& the_tree;

            /** Canonical link to symbol with lower ID, if any (i.e. known parent) */
            SymbolLink * canonical_origin = nullptr;

            /** First link, if any, to symbols with higher ID (i.e. children) */
            SymbolLink * first_link = nullptr;

            /** Final link, if any, to symbols with higher ID (i.e. children) */
            SymbolLink * last_link = nullptr;

        public:
            constexpr explicit SymbolNode(SymbolTree& tree, symbol_name_t name) noexcept : the_tree{tree}, Symbol{name} { }

            constexpr explicit SymbolNode(SymbolTree& tree, Symbol symbol) noexcept : the_tree{tree}, Symbol{symbol} { }

            [[nodiscard]] constexpr SymbolLinkIterator begin() noexcept {
                return SymbolLinkIterator{this->first_link};
            }

            [[nodiscard]] constexpr  SymbolLinkIterator end() noexcept {
                return SymbolLinkIterator{};
            }

            [[nodiscard]] constexpr SymbolLinkConstIterator begin() const noexcept {
                return this->cbegin();
            }

            [[nodiscard]] constexpr  SymbolLinkConstIterator end() const noexcept {
                return this->cend();
            }

            [[nodiscard]] constexpr SymbolLinkConstIterator cbegin() const noexcept {
                return SymbolLinkConstIterator{this->first_link};
            }

            [[nodiscard]] constexpr SymbolLinkConstIterator cend() const noexcept {
                return SymbolLinkConstIterator{};
            }

            [[nodiscard]] constexpr bool empty() const noexcept {
                return this->first_link == nullptr;
            }

            /**
             * Register a link with this node. Pushes link to the back, without checking order.
             * @param link Pointer to the SymbolLink object to incorporate.
             */
            void insert_back(SymbolLink * link) noexcept;

            /**
             * Register a link with this node, placing it in order of target id.
             * Does not check for duplication! Use insert_or_merge if this is required.
             * @param link Pointer to the SymbolLink object to incorporate.
             * @param hint Pointer to the first SymbolLink object in the node, to check if link is less than.
             * @return Pair: first value true if link was merged, second value points to link as in node.
             */
            std::pair<bool, SymbolLink*> insert_ordered(SymbolLink* link, SymbolLink * hint = nullptr) noexcept;


             /**
              * Absorb a link to a (canonical) node, inserting link and all sub-links into this node's link list.
              * @param source SymbolLink object that describes the relationship between this node and node to be absorbed.
              * @return Number of links added to this object.
              */
             size_t subsume(SymbolLink * source) noexcept;

            /**
             * Represents the lowest id symbol equivalent (up to negation / conjugation) to this node.
             */
             [[nodiscard]] SymbolExpression canonical_expression() const noexcept;

             /**
              * Returns a pair of this node, and its canonical equivalent, with equivalence information.
              */
             [[nodiscard]] SymbolPair canonical_pair() const noexcept;

             /**
              * @return True, if expression has no
              */
            [[nodiscard]] bool unaliased() const noexcept { return this->canonical_origin == nullptr; }

            friend class detail::SymbolNodeSimplifyImpl;


        };

    private:
        SymbolSet::packing_map_t packing_map;
        std::vector<symbol_name_t> unpacking_map;

        std::vector<SymbolNode> tree_nodes;
        std::vector<SymbolLink> tree_links;
        std::vector<SymbolLink*> available_links;
        bool done_simplification = false;
        size_t num_aliases = 0;

    public:
        /** Construct a symbol tree from a symbol set (copying un/packing maps) */
        explicit SymbolTree(const SymbolSet& symbols);

        /** Construct a symbol tree from a symbol set (moving un/packing maps) */
        explicit SymbolTree(SymbolSet&& symbols);

        /** SymbolTree has no copy constructor. */
        SymbolTree(const SymbolTree& rhs) = delete;

        /** The number of nodes in the tree */
        [[nodiscard]] size_t count_nodes() const noexcept { return tree_nodes.size(); }

        /** The number of initial links in the tree */
        [[nodiscard]] size_t max_links() const noexcept { return tree_links.size(); }

        /** The number of nodes that are not base nodes */
        [[nodiscard]] size_t alias_count() const noexcept {return this->num_aliases; }

        /** Execute the simplification algorithm */
        void simplify();

        /** True if the tree has been simplified */
        [[nodiscard]] bool ready() const { return done_simplification; }

        /** Gets the node at supplied index */
        const SymbolNode& operator[](size_t index) const {
            return this->tree_nodes[index];
        }

        /**
         * Use the solved tree to rewrite symbols in a canonical way
         * @param expr A symbol expression
         * @return The 'canonical' way of writing that symbolic expression.
         */
        [[nodiscard]] SymbolExpression substitute(SymbolExpression expr) const noexcept;

        /**
         * Copies the (solved) network back into a SymbolSet object.
         */
        [[nodiscard]] std::unique_ptr<SymbolSet> export_symbol_set() const;

        friend std::ostream& operator<<(std::ostream& os, const SymbolTree& st);

        friend class detail::SymbolNodeSimplifyImpl;

        friend class SymbolSet;

    protected:
        /** Empty constructor, for mock classes */
        SymbolTree();

    private:
        void make_nodes_and_links(const SymbolSet& symbols);

        /** Flags a SymbolLink as unused */
        void releaseLink(SymbolLink * link);

        /** Returns an unused SymbolLink */
        SymbolLink * getAvailableLink();

    };

}