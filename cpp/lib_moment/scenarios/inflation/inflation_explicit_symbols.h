/**
 * inflation_explicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "scenarios/explicit_symbols.h"
#include "observable_variant_index.h"

#include <span>
#include <vector>

namespace Moment {

    class InflationMatrixSystem;
    class CanonicalObservables;

    /** An index of explicit real operators, according to the parties and measurements chosen. */
    class InflationExplicitSymbolIndex : public ExplicitSymbolIndex {

    private:
        const CanonicalObservables& canonicalObservables;

        std::vector<ptrdiff_t> indices;

    public:
        /**
         * Construct explicit symbol table for inflation system
         */
        InflationExplicitSymbolIndex(const InflationMatrixSystem& ms, size_t level);

        using ExplicitSymbolIndex::get;

        [[nodiscard]] std::span<const ExplicitSymbolEntry> get(std::span<const size_t> mmtIndices) const override;

        [[nodiscard]] std::span<const ExplicitSymbolEntry> get(std::span<const OVIndex> indices) const;

        [[nodiscard]] inline std::span<const ExplicitSymbolEntry> get(std::initializer_list<OVIndex> mmtIndices) const {
            std::vector<OVIndex> v{mmtIndices};
            return get(v);
        }

    };
}