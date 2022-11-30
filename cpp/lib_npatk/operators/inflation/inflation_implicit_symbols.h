/**
 * inflation_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/common/implicit_symbols.h"

#include "inflation_context.h"

namespace NPATK {

    class InflationMatrixSystem;

    /**
     * Calculate the 'missing' marginals/probabilities from the explicit form.
     */
    class InflationImplicitSymbols : public ImplicitSymbols {
    public:
        const InflationContext &context;
    public:
        InflationImplicitSymbols(const InflationMatrixSystem& ims);

    };
}