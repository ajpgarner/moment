/**
 * inflation_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "inflation_matrix_system.h"

#include "canonical_observables.h"
#include "factor_table.h"
#include "inflation_context.h"
#include "inflation_explicit_symbols.h"
#include "inflation_implicit_symbols.h"


namespace NPATK {
    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class InflationContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
        this->canonicalObservables = std::make_unique<class CanonicalObservables>(this->inflationContext);
    }

    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class Context> contextIn)
            : MatrixSystem{std::move(contextIn)},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
        this->canonicalObservables = std::make_unique<class CanonicalObservables>(this->inflationContext);
    }

    InflationMatrixSystem::~InflationMatrixSystem() noexcept = default;

    const InflationExplicitSymbolIndex &InflationMatrixSystem::ExplicitSymbolTable() const {
        if (!this->explicitSymbols) {
            throw errors::missing_component("ExplicitSymbolTable has not yet been generated.");
        }
        return *this->explicitSymbols;
    }

    const InflationImplicitSymbols &InflationMatrixSystem::ImplicitSymbolTable() const {
        if (!this->implicitSymbols) {
            throw errors::missing_component("ImplicitSymbolTable has not yet been generated.");
        }
        return *this->implicitSymbols;
    }

    size_t InflationMatrixSystem::MaxRealSequenceLength() const noexcept {
        // Largest order of moment matrix?
        ptrdiff_t hierarchy_level = this->highest_moment_matrix();
        if (hierarchy_level < 0) {
            hierarchy_level = 0;
        }

        // Max sequence can't also be longer than number of observable variants
        return std::min(hierarchy_level*2, static_cast<ptrdiff_t>(this->inflationContext.observable_variant_count()));
    }


    void InflationMatrixSystem::onNewMomentMatrixCreated(size_t level, const class MomentMatrix& mm) {
        // Register factors
        this->factors->on_new_symbols_added();

        // Update canonical observables (if necessary)
        const auto new_max_length = this->MaxRealSequenceLength();
        this->canonicalObservables->generate_up_to_level(new_max_length);

        // Update explicit/implicit symbols
        if (!this->explicitSymbols || (this->explicitSymbols->Level < new_max_length)) {
            this->explicitSymbols = std::make_unique<InflationExplicitSymbolIndex>(*this, new_max_length);
            this->implicitSymbols = std::make_unique<InflationImplicitSymbols>(*this);
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