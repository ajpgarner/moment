/**
 * locality_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "locality_matrix_system.h"

#include "locality_context.h"
#include "locality_explicit_symbols.h"
#include "locality_implicit_symbols.h"
#include "collins_gisin.h"

#include "symbolic/monomial_comparator_by_hash.h"

namespace Moment::Locality {
    LocalityMatrixSystem::~LocalityMatrixSystem() noexcept = default;


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct Context> contextIn, const double tolerance)
            : MatrixSystem{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance, this->Symbols())
        );

        this->explicitSymbols = std::make_unique<LocalityExplicitSymbolIndex>(*this, 0);
        this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);
        this->collinsGisin = std::make_unique<Moment::Locality::CollinsGisin>(*this);
    }


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct LocalityContext> contextIn,  const double tolerance)
            : MatrixSystem{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance, this->Symbols())
        );

        this->explicitSymbols = std::make_unique<LocalityExplicitSymbolIndex>(*this, 0);
        this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);
        this->collinsGisin = std::make_unique<Moment::Locality::CollinsGisin>(*this);
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
            throw Moment::errors::missing_component("ExplicitSymbolTable has not yet been generated.");
        }
        return *this->explicitSymbols;
    }

    const LocalityImplicitSymbols& LocalityMatrixSystem::ImplicitSymbolTable() const {

        if (!this->implicitSymbols) {
            throw Moment::errors::missing_component("ImplicitSymbolTable has not yet been generated.");
        }
        return *this->implicitSymbols;
    }

    const class CollinsGisin& LocalityMatrixSystem::CollinsGisin() const {
        if (!this->collinsGisin) {
            throw Moment::errors::missing_component("Collins-Gisin tensor has not yet been generated. ");
        }
        return *this->collinsGisin;
    }

    void LocalityMatrixSystem::onNewMomentMatrixCreated(size_t level, const class Matrix &mm) {
        auto newMRSL = this->MaxRealSequenceLength();
        if (newMRSL > this->maxProbabilityLength) {
            this->maxProbabilityLength = newMRSL;

            // Make explicit/implicit symbol table
            this->explicitSymbols = std::make_unique<LocalityExplicitSymbolIndex>(*this, this->MaxRealSequenceLength());
            this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);

            // Fill CG tensor
            this->collinsGisin->fill_missing_symbols(this->Symbols());
        }
    }

    void LocalityMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const Matrix &lm) {
        this->collinsGisin->fill_missing_symbols(this->Symbols());
    }

    void LocalityMatrixSystem::onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) {
        if (word_length > this->maxProbabilityLength) {
            this->maxProbabilityLength = word_length;

            // Make explicit/implicit symbol table
            this->explicitSymbols = std::make_unique<LocalityExplicitSymbolIndex>(*this, this->MaxRealSequenceLength());
            this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);

            // Fill CG tensor
            this->collinsGisin->fill_missing_symbols(this->Symbols());
        }
    }


}