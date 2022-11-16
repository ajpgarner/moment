/**
 * inflation_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_matrix_system.h"
#include "inflation_context.h"

namespace NPATK {
    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class InflationContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {

    }

    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {

    }
}