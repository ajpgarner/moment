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

    private:
        equality_map_t symbol_links{};
        std::set<symbol_name_t> unique_names{};
        std::map<symbol_name_t, symbol_name_t> packing_key{};
        std::vector<symbol_name_t> unpacking_key{};

        bool packed = false;

    public:
        /**
         * @param raw_pairs Not necessarily unique list of symbolic pairs
         */
        explicit SymbolSet(const std::vector<SymbolPair>& raw_pairs);

        SymbolSet(SymbolSet&& rhs) = default;

        [[nodiscard]] size_t symbol_count() const noexcept { return unique_names.size(); }

        [[nodiscard]] size_t link_count() const noexcept { return symbol_links.size(); }

        [[nodiscard]] auto begin() const noexcept { return symbol_links.cbegin(); }

        [[nodiscard]] auto end() const noexcept { return symbol_links.cend(); }

        [[nodiscard]] const auto& unpacked_names() const noexcept { return unique_names; }

        [[nodiscard]] constexpr bool is_packed() const noexcept { return this->packed; }

        friend std::ostream& operator<<(std::ostream& os, const SymbolSet& symbolSet);

        /**
         * Re-labels nodes and links so that symbol names begin at 0, and contain no gaps.
         */
        void pack();

        /**
         * Restores original names of symbols
         */
        void unpack();

    };
}