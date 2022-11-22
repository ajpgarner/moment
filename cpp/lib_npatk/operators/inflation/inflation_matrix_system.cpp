/**
 * inflation_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_matrix_system.h"

#include "inflation_context.h"
#include "factor_table.h"


namespace NPATK {
    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class InflationContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
    }

    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
    }

    InflationMatrixSystem::~InflationMatrixSystem() noexcept = default;

    void InflationMatrixSystem::onNewMomentMatrixCreated(size_t level, const class MomentMatrix& mm) {
        this->factors->check_for_new_factors();
        MatrixSystem::onNewMomentMatrixCreated(level, mm);
    }

    void InflationMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi,
                                                             const class LocalizingMatrix& lm) {
        this->factors->check_for_new_factors();
        MatrixSystem::onNewLocalizingMatrixCreated(lmi, lm);
    }
}