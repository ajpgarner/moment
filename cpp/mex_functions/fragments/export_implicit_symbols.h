/**
 * export_implicit_symbols.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "mex.hpp"
#include "MatlabDataArray.hpp"

#include "symbolic/linear_combo.h"

namespace NPATK {
    class ImplicitSymbols;

    namespace mex {
        matlab::data::StructArray export_implied_symbols(matlab::engine::MATLABEngine &engine,
                                                   const ImplicitSymbols &impliedSymbols);


    }
}