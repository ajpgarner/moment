/**
 * inflation_implicit_symbols.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_implicit_symbols.h"

#include "inflation_explicit_symbols.h"
#include "inflation_matrix_system.h"

namespace NPATK {
    InflationImplicitSymbols::InflationImplicitSymbols(const InflationMatrixSystem& ms)
        : ImplicitSymbols{ms.Symbols(), ms.ExplicitSymbolTable(), ms.MaxRealSequenceLength()},
          context{ms.InflationContext()}  {

    }
}