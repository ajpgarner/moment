/**
 * inflation_explicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/common/explicit_symbols.h"

namespace NPATK {

    class InflationMatrixSystem;

    /** An index of explicit real operators, according to the parties and measurements chosen. */
    class InflationExplicitSymbolIndex : public ExplicitSymbolIndex {
    public:
        /**
         * Construct explicit symbol table for inflation system
         */
        InflationExplicitSymbolIndex(const InflationMatrixSystem& ms, size_t level);

    };
}