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
            : MaintainsTensors{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance, this->Symbols())
        );

        //this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);
    }


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct LocalityContext> contextIn,  const double tolerance)
            : MaintainsTensors{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance, this->Symbols())
        );

        //this->implicitSymbols = std::make_unique<LocalityImplicitSymbols>(*this);
    }

    size_t LocalityMatrixSystem::MaxRealSequenceLength() const noexcept {

        // Largest order of moment matrix?
        ptrdiff_t hierarchy_level = this->MomentMatrix.Indices().highest();
        if (hierarchy_level < 0) {
            hierarchy_level = 0;
        }

        // Max sequence can't also be longer than number of parties
        return std::min(hierarchy_level*2, static_cast<ptrdiff_t>(this->localityContext.Parties.size()));
    }

    const class LocalityProbabilityTensor& LocalityMatrixSystem::LocalityProbabilityTensor() const {
        const auto& pt = this->ProbabilityTensor();
        return dynamic_cast<const class LocalityProbabilityTensor&>(pt);
    }

    const class LocalityCollinsGisin& LocalityMatrixSystem::LocalityCollinsGisin() const {
        const auto& cg = this->CollinsGisin();
        return dynamic_cast<const class LocalityCollinsGisin&>(cg);
    }

    void LocalityMatrixSystem::onNewMomentMatrixCreated(size_t level, const class Matrix &mm) {
        auto newMRSL = this->MaxRealSequenceLength();
        if (newMRSL > this->maxProbabilityLength) {
            this->maxProbabilityLength = newMRSL;
        }
    }

    void LocalityMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi, const Matrix &lm) {

    }

    void LocalityMatrixSystem::onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) {
        if (word_length > this->maxProbabilityLength) {
            this->maxProbabilityLength = word_length;

        }
    }

    std::unique_ptr<class CollinsGisin> LocalityMatrixSystem::makeCollinsGisin() {
        return std::make_unique<class LocalityCollinsGisin>(*this);
    }

    std::unique_ptr<class ProbabilityTensor> LocalityMatrixSystem::makeProbabilityTensor() {
        return std::make_unique<class LocalityProbabilityTensor>(*this);
    }


}