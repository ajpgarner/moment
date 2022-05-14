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
         * @param to_add The symbol to be added
         * @return true if a new symbol, false if existing symbol was changed.
         */
        bool add_or_merge(const Symbol& to_add);

        /**
         * Re-labels nodes and links so that symbol names begin at 0, and contain no gaps.
         */
        void pack();

        /**
         * Restores original names of symbols
         */
        void unpack();

        friend std::ostream& operator<<(std::ostream& os, const SymbolSet& symbolSet);

    };
}