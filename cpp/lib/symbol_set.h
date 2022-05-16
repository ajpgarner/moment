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
#include "equality_type.h"

namespace NPATK {

    class SymbolSet {
    public:


        using equality_map_t = std::map<std::pair<symbol_name_t, symbol_name_t>, EqualityType>;
        using symbol_map_t = std::map<symbol_name_t, Symbol>;


        struct LinkRange {
        private:
            const SymbolSet& the_set;
        public:
            constexpr explicit LinkRange(const SymbolSet& ss) noexcept : the_set{ss} { }

            [[nodiscard]] auto begin() const noexcept { return the_set.symbol_links.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return the_set.symbol_links.cend(); }
        };

        struct SymbolRange {
        private:
            const SymbolSet& the_set;
        public:
            constexpr explicit SymbolRange(const SymbolSet& ss) noexcept  : the_set{ss} { }

            [[nodiscard]] auto begin() const noexcept { return the_set.symbols.cbegin(); }
            [[nodiscard]] auto end() const noexcept { return the_set.symbols.cend(); }
        };

    private:
        symbol_map_t symbols{};
        equality_map_t symbol_links{};

        std::map<symbol_name_t, symbol_name_t> packing_key{};
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
        /**
         * @param raw_pairs Not necessarily unique list of symbolic pairs
         */
        explicit SymbolSet(const std::vector<SymbolPair>& raw_pairs);

        /**
         * @param raw_pairs Not necessarily unique list of symbolic pairs
         */
        SymbolSet(const std::vector<Symbol>& extra_symbols, const std::vector<SymbolPair>& raw_pairs);

        SymbolSet(const SymbolSet& rhs) = delete;

        SymbolSet(SymbolSet&& rhs) noexcept : Links{*this}, Symbols{*this},
            symbols{std::move(rhs.symbols)}, symbol_links{std::move(rhs.symbol_links)},
            packing_key{std::move(rhs.packing_key)}, unpacking_key{std::move(rhs.unpacking_key)},
            packed{rhs.packed} { }

        [[nodiscard]] size_t symbol_count() const noexcept { return symbols.size(); }

        [[nodiscard]] size_t link_count() const noexcept { return symbol_links.size(); }

        [[nodiscard]] constexpr bool is_packed() const noexcept { return this->packed; }

        /**
         * Adds symbol, or applies constraints (e.g. realness) from symbol to existing symbol in set.
         * @param to_add The symbol to be added.
         * @return true if a new symbol, false if existing symbol was changed.
         */
        bool add_or_merge(const Symbol& to_add);

        /**
         * Re-labels nodes and links so that symbol names begin at 0, and contain no gaps.
         */
        void pack();

        /**
         * Restores original names of symbols.
         */
        void unpack();

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

    };
}