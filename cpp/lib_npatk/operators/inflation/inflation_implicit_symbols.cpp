/**
 * inflation_implicit_symbols.cpp
 *
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_implicit_symbols.h"

#include "inflation_matrix_system.h"

namespace NPATK {
    InflationImplicitSymbols::InflationImplicitSymbols(const InflationMatrixSystem& ms)
        : ImplicitSymbols{ms.Symbols(), ms.ExplicitSymbolTable(), ms.MaxRealSequenceLength(),
                          JointMeasurementIndex{std::vector<size_t>(ms.InflationContext().Observables().size(), 1),
                                                std::min(ms.InflationContext().Observables().size(),
                                                         ms.MaxRealSequenceLength())}},
        context{ms.InflationContext()}  {

    }
}