/**
 * export_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/linear_combo.h"
#include "operators/inflation/observable_variant_index.h"
#include "operators/locality/measurement.h"

#include <span>

namespace NPATK {
    class Context;
    class MomentMatrix;
    class LocalityImplicitSymbols;
    class InflationImplicitSymbols;
    class PMODefinition;

    namespace mex {
        /**
         * Export complete table of implied symbols from inflation scenario
         */
        [[nodiscard]] matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                                       const InflationImplicitSymbols &impliedSymbols);

        /**
         * Export one observable of implied symbols from inflation scenario
         */
        [[nodiscard]] matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                                       const InflationImplicitSymbols &impliedSymbols,
                                                                       std::span<const OVIndex> obsVarIndices);

        /**
         * Export complete table of implied symbols from locality scenario
         */
        [[nodiscard]] matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                                       const LocalityImplicitSymbols &impliedSymbols);

        /**
         * Export one measurement of implied symbols from inflation scenario
         */
        [[nodiscard]] matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                                       const LocalityImplicitSymbols &impliedSymbols,
                                                                       std::span<const PMIndex> measurementIndex);

        matlab::data::StructArray export_implied_symbol_row(matlab::engine::MATLABEngine &engine,
                                                            const MomentMatrix& momentMatrix,
                                                            const std::vector<PMOIndex>& pmoIndices,
                                                            const PMODefinition& impliedSymbols);
    }
}