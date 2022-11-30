/**
 * explicit_symbol.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "operators/common/explicit_symbols.h"

namespace NPATK {

    class LocalityMatrixSystem;

    /** An index of explicit real operators, according to the parties and measurements chosen. */
    class LocalityExplicitSymbolIndex : public ExplicitSymbolIndex {
    public:
        /**
         * Construct explicit symbol table for locality system
         */
        LocalityExplicitSymbolIndex(const LocalityMatrixSystem& ms, size_t level);

    };
}