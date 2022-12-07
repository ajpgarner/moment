/**
 * inflation_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/common/implicit_symbols.h"

#include "inflation_context.h"

#include <vector>

namespace NPATK {

    class InflationMatrixSystem;
    class InflationExplicitSymbolIndex;
    class CanonicalObservable;
    class CanonicalObservables;

    /**
     * Calculate the 'missing' marginals/probabilities from the explicit form.
     */
    class InflationImplicitSymbols : public ImplicitSymbols {
    public:
        const InflationContext &context;
        const CanonicalObservables& canonicalObservables;
    private:
        std::vector<ptrdiff_t> indices;
        const InflationExplicitSymbolIndex& iesi;

    public:
        explicit InflationImplicitSymbols(const InflationMatrixSystem& ims);

        using ImplicitSymbols::get;

        [[nodiscard]] std::span<const PMODefinition> get(std::span<const size_t> mmtIndex) const override;

        [[nodiscard]] std::span<const PMODefinition> get(std::span<const OVIndex> mmtIndices) const;

    private:
        size_t generateFromCanonicalObservable(const CanonicalObservable& canonicalObservable);
        size_t generateLevelZero(const CanonicalObservable& canonicalObservable);
        size_t generateLevelOne(const CanonicalObservable& canonicalObservable);
        size_t generateMoreLevels(const CanonicalObservable& canonicalObservable);

    };
}