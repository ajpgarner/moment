/**
 * locality_matrix_system.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "locality_matrix_system.h"

#include "locality_context.h"
#include "explicit_symbols.h"
#include "implicit_symbols.h"
#include "collins_gisin.h"

namespace NPATK {
    LocalityMatrixSystem::~LocalityMatrixSystem() noexcept = default;


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct Context> contextIn)
            : MatrixSystem(std::move(contextIn)),
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->explicitSymbols = std::make_unique<ExplicitSymbolIndex>(*this, 0);
        this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);
    }


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct LocalityContext> contextIn)
            : MatrixSystem{std::move(contextIn)},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->explicitSymbols = std::make_unique<ExplicitSymbolIndex>(*this, 0);
        this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);
    }

    size_t LocalityMatrixSystem::MaxRealSequenceLength() const noexcept {

        // Largest order of moment matrix?
        ptrdiff_t hierarchy_level = this->highest_moment_matrix();
        if (hierarchy_level < 0) {
            hierarchy_level = 0;
        }

        // Max sequence can't also be longer than number of parties
        return std::min(hierarchy_level*2, static_cast<ptrdiff_t>(this->localityContext.Parties.size()));
    }

    const ExplicitSymbolIndex& LocalityMatrixSystem::ExplicitSymbolTable() const {

        if (!this->explicitSymbols) {
            throw errors::missing_component("ExplicitSymbolTable has not yet been generated.");
        }
        return *this->explicitSymbols;
    }

    const ImplicitSymbols& LocalityMatrixSystem::ImplicitSymbolTable() const {

        if (!this->implicitSymbols) {
            throw errors::missing_component("ImplicitSymbolTable has not yet been generated.");
        }
        return *this->implicitSymbols;
    }

    const class CollinsGisin& LocalityMatrixSystem::CollinsGisin() const {
        if (!this->collinsGisin) {
            throw errors::missing_component(std::string("Collins-Gisin tensor has not yet been generated. ")
                                            + "Perhaps a large enough moment matrix has not yet been created.");
        }
        return *this->collinsGisin;
    }

    void LocalityMatrixSystem::onNewMomentMatrixCreated(size_t level, const class MomentMatrix &mm) {
        auto newMRSL = this->MaxRealSequenceLength();
        if (newMRSL > this->maxProbabilityLength) {
            this->maxProbabilityLength = newMRSL;

            // Make explicit/implicit symbol table
            this->explicitSymbols = std::make_unique<ExplicitSymbolIndex>(*this, this->MaxRealSequenceLength());
            this->implicitSymbols = std::make_unique<ImplicitSymbols>(*this);

            // Can/should we make C-G tensor?
            if (!this->collinsGisin && (newMRSL >= this->localityContext.Parties.size())) {
                this->collinsGisin = std::make_unique<NPATK::CollinsGisin>(*this);
            }
        }
    }



}