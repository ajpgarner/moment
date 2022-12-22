/**
 * explicit_symbol.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "scenarios/explicit_symbols.h"

namespace Moment::Locality {

    class LocalityMatrixSystem;

    /** An index of explicit real operators, according to the parties and measurements chosen. */
    class LocalityExplicitSymbolIndex : public ExplicitSymbolIndex {
    private:
        JointMeasurementIndex indices;

    public:
        /**
         * Construct explicit symbol table for locality system
         */
        LocalityExplicitSymbolIndex(const LocalityMatrixSystem& ms, size_t level);

        [[nodiscard]] std::span<const ExplicitSymbolEntry> get(std::span<const size_t> mmtIndices) const override;
    };
}