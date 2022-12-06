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
    class CanonicalObservable;

    /**
     * Calculate the 'missing' marginals/probabilities from the explicit form.
     */
    class InflationImplicitSymbols : public ImplicitSymbols {
    public:
        const InflationContext &context;
    private:
        std::vector<size_t> indices;

    public:
        explicit InflationImplicitSymbols(const InflationMatrixSystem& ims);

    private:
        size_t generateFromCanonicalObservable(const CanonicalObservable& canonicalObservable);

    };
}