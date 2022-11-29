/**
 * inflation_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_matrix_system.h"

#include "inflation_context.h"
#include "factor_table.h"

#include "operators/locality/explicit_symbols.h"


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

    const ExplicitSymbolIndex &InflationMatrixSystem::ExplicitSymbolTable() const {
        if (!this->explicitSymbols) {
            throw errors::missing_component("ExplicitSymbolTable has not yet been generated.");
        }
        return *this->explicitSymbols;
    }

    size_t InflationMatrixSystem::MaxRealSequenceLength() const noexcept {
        // Largest order of moment matrix?
        ptrdiff_t hierarchy_level = this->highest_moment_matrix();
        if (hierarchy_level < 0) {
            hierarchy_level = 0;
        }

        // Max sequence can't also be longer than number of observables
        return std::min(hierarchy_level*2, static_cast<ptrdiff_t>(this->inflationContext.Observables().size()));
    }


    void InflationMatrixSystem::onNewMomentMatrixCreated(size_t level, const class MomentMatrix& mm) {
        // Register factors
        this->factors->on_new_symbols_added();

        // Update explicit symbols
        const auto new_max_length = this->MaxRealSequenceLength();
        if (!this->explicitSymbols || (this->explicitSymbols->Level < new_max_length)) {
            this->explicitSymbols = std::make_unique<ExplicitSymbolIndex>(*this, new_max_length);
        }

        MatrixSystem::onNewMomentMatrixCreated(level, mm);
    }

    void InflationMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi,
                                                             const class LocalizingMatrix& lm) {
        // Register factors
        this->factors->on_new_symbols_added();
        MatrixSystem::onNewLocalizingMatrixCreated(lmi, lm);
    }

}