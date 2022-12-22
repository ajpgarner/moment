/**
 * inflation_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "scenarios/implicit_symbols.h"

#include "inflation_context.h"

#include "utilities/variable_chunk_range.h"

#include <vector>

namespace Moment::Inflation {

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

        [[nodiscard]] VariableChunkRange<PMODefinition, ptrdiff_t> BlockData() const noexcept {
            return {this->tableData, this->indices};
        }

        [[nodiscard]] std::span<const PMODefinition> Block(size_t index) const noexcept;


    private:
        size_t generateFromCanonicalObservable(const CanonicalObservable& canonicalObservable);
        size_t generateLevelZero(const CanonicalObservable& canonicalObservable);
        size_t generateLevelOne(const CanonicalObservable& canonicalObservable);
        size_t generateMoreLevels(const CanonicalObservable& canonicalObservable);

    };
}