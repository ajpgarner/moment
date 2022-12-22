/**
 * symbol_set.h
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include <cstdint>
#include <iosfwd>
#include <map>
#include <set>

#include "symbol.h"
#include "symbol_expression.h"
#include "equality_type.h"

namespace Moment {

    class SymbolTree;

    class SymbolSet {
    public:
        using equality_map_t = std::map<std::pair<symbol_name_t, symbol_name_t>, EqualityType>;
        using symbol_map_t = std::map<symbol_name_t, Symbol>;
        using packing_map_t = std::map<symbol_name_t, symbol_name_t>;

        struct LinkRange {
        private:
            const SymbolSet& the_set;
        public:
            constexpr explicit LinkRange(const SymbolSet& ss) noexcept : the_set{ss} { }

            [[nodiscard]] auto begin() const noexcept { return the_set.symbol_links.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return the_set.symbol_links.cend(); }
            [[nodiscard]] size_t size() const noexcept { return the_set.symbol_links.size(); }
        };

        struct SymbolRange {
        private:
            const SymbolSet& the_set;
        public:
            constexpr explicit SymbolRange(const SymbolSet& ss) noexcept  : the_set{ss} { }

            [[nodiscard]] auto begin() const noexcept { return the_set.symbols.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return the_set.symbols.cend(); }
            [[nodiscard]] size_t size() const noexcept { return the_set.symbols.size(); }
        };

    private:
        symbol_map_t symbols{};
        equality_map_t symbol_links{};

        packing_map_t packing_key{};
        std::vector<symbol_name_t> unpacking_key{};

        bool packed = false;

    public:
        /**
         * Range, over all links in SymbolSet.
         */
        LinkRange Links;

        /**
         * Range, over all symbols in SymbolSet.
         */
        SymbolRange Symbols;

    public:
        SymbolSet();

        /**
         * @param symbols List of (not necessarily unique) symbols.
         */
        explicit SymbolSet(const std::vector<Symbol>& symbols);

        /**
         * @param raw_pairs Not necessarily unique list of symbolic pairs
         */
        explicit SymbolSet(const std::vector<SymbolPair>& raw_pairs);

        /**
         * @param extra_symbols List of (not necessarily unique) symbols.
         * @param raw_pairs Not necessarily unique list of symbolic pairs
         */
        SymbolSet(const std::vector<Symbol>& extra_symbols, const std::vector<SymbolPair>& raw_pairs);

        /**
         * Extract a set from a SymbolTree..
         * @param tree The SymbolTree
         */
        explicit SymbolSet(const SymbolTree& tree);

        SymbolSet(const SymbolSet& rhs) = delete;


        SymbolSet(SymbolSet&& rhs) noexcept : Links{*this}, Symbols{*this},
            symbols{std::move(rhs.symbols)}, symbol_links{std::move(rhs.symbol_links)},
            packing_key{std::move(rhs.packing_key)}, unpacking_key{std::move(rhs.unpacking_key)},
            packed{rhs.packed} { }

        SymbolSet& operator=(SymbolSet&& rhs) noexcept {
            std::swap(this->symbols, rhs.symbols);
            std::swap(this->symbol_links, rhs.symbol_links);
            std::swap(this->packing_key, rhs.packing_key);
            std::swap(this->unpacking_key, rhs.unpacking_key);
            packed = rhs.packed;
            return *this;
        }

        [[nodiscard]] size_t symbol_count() const noexcept { return symbols.size(); }

        [[nodiscard]] size_t link_count() const noexcept { return symbol_links.size(); }

        [[nodiscard]] constexpr bool is_packed() const noexcept { return this->packed; }

        /**
         * Adds symbol, or applies constraints (e.g. realness) from symbol to existing symbol in set.
         * @param to_add The symbol to be added.
         * @return true if a new symbol was inserted.
         */
        bool add_or_merge(const Symbol& to_add);

        /**
         * Adds an equation between two symbols, supplying the symbols to the set if not already included.
         * @param to_add The symbol pair to be added
         * @param force_real true if the symbols are real (false for possibly complex).
         * @return true if a new pair was added.
         */
        bool add_or_merge(const SymbolPair& to_add, bool force_real = false);


        /**
         * Re-labels nodes and links so that symbol names begin at 0, and contain no gaps.
         */
        void pack();

        /**
         * Restores original names of symbols.
         */
        void unpack();

        /**
         * Wipe everything
         */
        void reset() noexcept;

        /**
         * Get the compressed element id, looking it up by its uncompressed key.
         * @param unpacked_key The element id, as it was before compression.
         * @return Pair, first: true if key was found, second: the id of the element after compression (if found).
         */
        [[nodiscard]] std::pair<bool, symbol_name_t> packed_key(symbol_name_t unpacked_key) const noexcept {
            auto iter = this->packing_key.find(unpacked_key);
            if (iter == this->packing_key.cend()) {
                return {false, 0};
            }
            return {true, iter->second};
        }

        /**
         * Get the uncompressed element id, looking it up by its compressed key.
         * @param packed_key The element id, as it is after compression
         * @return Pair, first: true if key was found, second: the id of the element before compression (if found).
         */
        [[nodiscard]] std::pair<bool, symbol_name_t> unpacked_key(symbol_name_t packed_key) const noexcept {
            if ((packed_key < 0) || (packed_key >= this->unpacking_key.size())) {
                return {false, 0};
            }
            return {true, this->unpacking_key[packed_key]};
        }

        friend std::ostream& operator<<(std::ostream& os, const SymbolSet& symbolSet);

        friend class SymbolTree;


    };
}