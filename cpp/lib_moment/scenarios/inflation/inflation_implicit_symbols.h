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

        /**
         * Get range over data, allowing iteration over each canonical measurement at a time.
         * May be invalidated if PMODefinition data is manipulated.
         */
        [[nodiscard]] VariableChunkRange<PMODefinition, ptrdiff_t> BlockData() const noexcept {
            return {this->tableData, this->indices};
        }

        /**
         * Get span of data regarding specific implicit measurement.
         * @param index The index of the canonical observable to get data on.
         * @return Span of PMODefinition data (i.e. linear expressions for each outcome in observable).
         */
        [[nodiscard]] std::span<const PMODefinition> Block(size_t index) const noexcept;

        using ImplicitSymbols::implicit_to_explicit;

        /**
         * Convert a full probability distribution over supplied (joint) observables to a list of explicit symbol
         * assignments imposing the same distribution.
         * @param mmtIndices The indices of the observable/variants to use
         * @param input_values The full probability distribution including implicit symbols
         * @return Vector of pairs, first being symbol ID, second being the calculated value, defining input explicitly.
         */
        std::map<symbol_name_t, double>
        implicit_to_explicit(std::span<const OVIndex> mmtIndices, std::span<const double> input_values) const;

        /**
         * Convert a full probability distribution over supplied (joint) observables to a list of explicit symbol
         * assignments imposing the same distribution.
         * @param canonicalObservable The observable to use
         * @param input_values The full probability distribution including implicit symbols
         * @return Vector of pairs, first being symbol ID, second being the calculated value, defining input explicitly.
         */
        std::map<symbol_name_t, double>
        implicit_to_explicit(const CanonicalObservable& canonicalObservable, std::span<const double> input_values) const;


    private:
        size_t generateFromCanonicalObservable(const CanonicalObservable& canonicalObservable);
        size_t generateLevelZero(const CanonicalObservable& canonicalObservable);
        size_t generateLevelOne(const CanonicalObservable& canonicalObservable);
        size_t generateMoreLevels(const CanonicalObservable& canonicalObservable);

    };
}