/**
 * locality_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "locality_matrix_system.h"

#include "locality_collins_gisin.h"
#include "locality_context.h"
#include "locality_full_correlator.h"
#include "locality_probability_tensor.h"

#include "symbolic/monomial_comparator_by_hash.h"

#include <algorithm>

namespace Moment::Locality {
    LocalityMatrixSystem::~LocalityMatrixSystem() noexcept = default;


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct Context> contextIn, const double tolerance)
            : MaintainsTensors{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance)
        );
    }


    LocalityMatrixSystem::LocalityMatrixSystem(std::unique_ptr<struct LocalityContext> contextIn,  const double tolerance)
            : MaintainsTensors{std::move(contextIn), tolerance},
              localityContext{dynamic_cast<const LocalityContext&>(this->Context())} {

        this->replace_polynomial_factory(
                std::make_unique<ByHashPolynomialFactory>(this->Symbols(), tolerance)
        );
    }

    const class LocalityCollinsGisin& LocalityMatrixSystem::LocalityCollinsGisin() const {
        const auto& cg = this->CollinsGisin();
        return dynamic_cast<const class LocalityCollinsGisin&>(cg);
    }

    const class LocalityFullCorrelator& LocalityMatrixSystem::LocalityFullCorrelator() const {
        const auto& fc = this->FullCorrelator();
        return dynamic_cast<const class LocalityFullCorrelator&>(fc);
    }

    const class LocalityProbabilityTensor& LocalityMatrixSystem::LocalityProbabilityTensor() const {
        const auto& pt = this->ProbabilityTensor();
        return dynamic_cast<const class LocalityProbabilityTensor&>(pt);
    }


    std::unique_ptr<class CollinsGisin> LocalityMatrixSystem::makeCollinsGisin() {
        return std::make_unique<class LocalityCollinsGisin>(*this);
    }

    std::unique_ptr<class FullCorrelator> LocalityMatrixSystem::makeFullCorrelator() {
        return std::make_unique<class LocalityFullCorrelator>(*this);
    }


    std::unique_ptr<class ProbabilityTensor> LocalityMatrixSystem::makeProbabilityTensor() {
        return std::make_unique<class LocalityProbabilityTensor>(*this);
    }

    bool LocalityMatrixSystem::CanHaveFullCorrelator() const noexcept {
        return std::all_of(this->localityContext.Parties.begin(), this->localityContext.Parties.end(),
                           [](const Party& party) {
           return std::all_of(party.Measurements.begin(), party.Measurements.end(), [](const Measurement& mmt) {
               return mmt.num_outcomes == 2;
           });
        });
    }


}