/**
 * export_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/linear_combo.h"
#include "operators/measurement.h"

#include <span>

namespace NPATK {
    class MomentMatrix;
    class ImplicitSymbols;
    class PMODefinition;

    namespace mex {
        matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                   const ImplicitSymbols &impliedSymbols);

        matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                   const ImplicitSymbols &impliedSymbols,
                                                   std::span<const PMIndex> measurementIndex);

        matlab::data::StructArray export_implied_symbol_row(matlab::engine::MATLABEngine &engine,
                                                            const MomentMatrix& momentMatrix,
                                                            const std::vector<PMOIndex>& pmoIndices,
                                                            const PMODefinition& impliedSymbols);
    }
}