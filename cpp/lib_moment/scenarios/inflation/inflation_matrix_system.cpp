/**
 * inflation_matrix_system.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "inflation_matrix_system.h"

#include "factor_table.h"
#include "extension_suggester.h"

#include "extended_matrix.h"
#include "inflation_collins_gisin.h"
#include "inflation_context.h"
#include "inflation_probability_tensor.h"


#include "matrix/symbolic_matrix.h"
#include "matrix/monomial_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"


namespace Moment::Inflation {
    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class InflationContext> contextIn,
                                                 const double zero_tolerance)
            : MaintainsTensors{std::move(contextIn), zero_tolerance},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())}, ExtendedMatrices{*this} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
        this->extensionSuggester = std::make_unique<ExtensionSuggester>(this->inflationContext,
                                                                        this->Symbols(), *this->factors);
    }

    InflationMatrixSystem::InflationMatrixSystem(std::unique_ptr<class Context> contextIn, const double zero_tolerance)
            : MaintainsTensors{std::move(contextIn), zero_tolerance},
              inflationContext{dynamic_cast<class InflationContext&>(this->Context())}, ExtendedMatrices{*this} {
        this->factors = std::make_unique<FactorTable>(this->inflationContext, this->Symbols());
        this->extensionSuggester = std::make_unique<ExtensionSuggester>(this->inflationContext,
                                                                        this->Symbols(), *this->factors);
    }

    InflationMatrixSystem::~InflationMatrixSystem() noexcept = default;

    void InflationMatrixSystem::onNewMomentMatrixCreated(size_t level, const SymbolicMatrix& mm) {
        // Register factors
        this->factors->on_new_symbols_added();
        MatrixSystem::onNewMomentMatrixCreated(level, mm);
    }

    void InflationMatrixSystem::onNewLocalizingMatrixCreated(const LocalizingMatrixIndex &lmi,
                                                             const SymbolicMatrix& lm) {
        // Register factors
        this->factors->on_new_symbols_added();
        MatrixSystem::onNewLocalizingMatrixCreated(lmi, lm);
    }

    std::unique_ptr<class ExtendedMatrix>
    InflationMatrixSystem::createNewExtendedMatrix(MaintainsMutex::WriteLock &lock, const ExtendedMatrixIndex &index,
                                                   const Multithreading::MultiThreadPolicy mt_policy) {

        // Get source moment matrix (or create it under lock)
        auto [source_index, source] = this->MomentMatrix.create(lock, index.moment_matrix_level, mt_policy);
        if (!source.is_monomial()) [[unlikely]] {
            throw std::logic_error{"Cannot extend non-monomial moment matrices."};
        }
        auto& monomial_source = dynamic_cast<MonomialMatrix&>(source);

        return std::make_unique<ExtendedMatrix>(this->Symbols(), this->Factors(),
                                                this->polynomial_factory().zero_tolerance,
                                                monomial_source, index.extension_list,
                                                mt_policy);
    }

    void InflationMatrixSystem::onNewExtendedMatrixCreated(const ExtendedMatrixIndex &, const ExtendedMatrix &em) {

    }

    std::set<symbol_name_t> InflationMatrixSystem::suggest_extensions(const class MonomialMatrix& matrix) const {
        return (*this->extensionSuggester)(matrix);
    }

    void InflationMatrixSystem::onDictionaryGenerated(size_t word_length, const OperatorSequenceGenerator &osg) {
        this->factors->on_new_symbols_added();
        MatrixSystem::onDictionaryGenerated(word_length, osg);
    }

    const class InflationProbabilityTensor& InflationMatrixSystem::InflationProbabilityTensor() const {
        auto& pt = this->ProbabilityTensor();
        return dynamic_cast<const class InflationProbabilityTensor&>(pt);
    }

    const class InflationCollinsGisin& InflationMatrixSystem::InflationCollinsGisin() const {
        auto& cg = this->CollinsGisin();
        return dynamic_cast<const class InflationCollinsGisin&>(cg);
    }

    std::unique_ptr<class CollinsGisin> InflationMatrixSystem::makeCollinsGisin() {
        return std::make_unique<class InflationCollinsGisin>(*this);
    }

    std::unique_ptr<class ProbabilityTensor> InflationMatrixSystem::makeProbabilityTensor() {
        return std::make_unique<class InflationProbabilityTensor>(*this);
    }

}