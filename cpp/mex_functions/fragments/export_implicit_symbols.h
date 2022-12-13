/**
 * export_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/linear_combo.h"
#include "operators/locality/measurement.h"

#include <span>

namespace NPATK {
    class Context;
    class MomentMatrix;
    class LocalityImplicitSymbols;
    class InflationImplicitSymbols;
    class PMODefinition;

    namespace mex {
        matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                         const InflationImplicitSymbols &impliedSymbols);

        matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                   const LocalityImplicitSymbols &impliedSymbols);

        matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                   const LocalityImplicitSymbols &impliedSymbols,
                                                   std::span<const PMIndex> measurementIndex);

        matlab::data::StructArray export_implied_symbol_row(matlab::engine::MATLABEngine &engine,
                                                            const MomentMatrix& momentMatrix,
                                                            const std::vector<PMOIndex>& pmoIndices,
                                                            const PMODefinition& impliedSymbols);
    }
}