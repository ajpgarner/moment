/**
 * locality_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "locality_matrix_system.h"

#include "locality_collins_gisin.h"
#include "locality_context.h"
#include "locality_probability_tensor.h"

#include "symbolic/monomial_comparator_by_hash.h"

namespace Moment::Locality {
    LocalityMatrixSystem::~LocalityMatrixSystem() noexcept = default;


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct Context> contextIn, const double tolerance)
            : MatrixSystem{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance, this->Symbols())
        );

        //this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);
    }


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct LocalityContext> contextIn,  const double tolerance)
            : MatrixSystem{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance, this->Symbols())
        );

        //this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);
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

    bool LocalityMatrixSystem::RefreshCollinsGisin(std::shared_lock<std::shared_mutex>& read_lock) {
        // First, if no explicit symbol table at all, we surely need to do something
        if (!this->collinsGisin) {
            read_lock.unlock();

            auto write_lock = this->get_write_lock();
            this->collinsGisin = std::make_unique<class Moment::Locality::LocalityCollinsGisin>(*this);
            const bool has_all_symbols = this->collinsGisin->HasAllSymbols();
            write_lock.unlock();

            read_lock.lock();
            return has_all_symbols;
        }

        // No missing symbols, return without ever having released read lock
        if (this->collinsGisin->HasAllSymbols()) {
            return true;
        }

        // Upgrade lock
        read_lock.unlock();
        auto write_lock = this->get_write_lock();

        // Try to fill symbols
        const bool filled = this->collinsGisin->fill_missing_symbols();

        // Downgrade lock
        write_lock.unlock();
        read_lock.lock();
        return filled;
    }

    bool LocalityMatrixSystem::RefreshCollinsGisin() {
        auto lock = this->get_read_lock();
        return this->RefreshCollinsGisin(lock);
    }

    const class ProbabilityTensor &LocalityMatrixSystem::ProbabilityTensor() const {
        if (!this->probabilityTensor) {
            throw Moment::errors::missing_component("ProbabilityTensor has not yet been generated.");
        }
        return *this->probabilityTensor;
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

            // Fill CG tensor
            //this->collinsGisin->fill_missing_symbols(this->Symbols());

            // Make explicit/implicit symbol table
            //this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);

        }
    }

    void LocalityMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const Matrix &lm) {

    }

    void LocalityMatrixSystem::onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) {
        if (word_length > this->maxProbabilityLength) {
            this->maxProbabilityLength = word_length;

        }
    }


}