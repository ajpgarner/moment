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
#include "inflation_full_correlator.h"
#include "inflation_probability_tensor.h"

#include "matrix/monomial_matrix.h"
#include "matrix/operator_matrix/moment_matrix.h"

#include "symbolic/rules/moment_rulebook.h"


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

    void InflationMatrixSystem::onNewSymbolsRegistered(const MaintainsMutex::WriteLock& write_lock,
                                                       size_t old_symbol_count, size_t new_symbol_count) {
        assert(write_lock.owns_lock());
        this->factors->on_new_symbols_added();
        this->Rulebook.refreshAll(write_lock, old_symbol_count);
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

    std::set<symbol_name_t> InflationMatrixSystem::suggest_extensions(const class MonomialMatrix& matrix) const {
        return (*this->extensionSuggester)(matrix);
    }


    const class InflationCollinsGisin& InflationMatrixSystem::InflationCollinsGisin() const {
        auto& cg = this->CollinsGisin();
        return dynamic_cast<const class InflationCollinsGisin&>(cg);
    }

    const class InflationFullCorrelator& InflationMatrixSystem::InflationFullCorrelator() const {
        auto& fc = this->FullCorrelator();
        return dynamic_cast<const class InflationFullCorrelator&>(fc);
    }

    const class InflationProbabilityTensor& InflationMatrixSystem::InflationProbabilityTensor() const {
        auto& pt = this->ProbabilityTensor();
        return dynamic_cast<const class InflationProbabilityTensor&>(pt);
    }

    std::unique_ptr<class CollinsGisin> InflationMatrixSystem::makeCollinsGisin() {
        return std::make_unique<class InflationCollinsGisin>(*this);
    }

    std::unique_ptr<class FullCorrelator> InflationMatrixSystem::makeFullCorrelator() {
        return std::make_unique<class InflationFullCorrelator>(*this);
    }

    std::unique_ptr<class ProbabilityTensor> InflationMatrixSystem::makeProbabilityTensor() {
        return std::make_unique<class InflationProbabilityTensor>(*this);
    }


    void InflationMatrixSystem::onRulebookAdded(const MaintainsMutex::WriteLock &write_lock, size_t index,
                                                const MomentRulebook &rb, bool insertion) {
        // Add additional rules to factor rulebook
        auto& write_rb = this->Rulebook(index);
        assert(&write_rb == &rb);
        this->expandRulebook(write_rb, 0);
    }

    ptrdiff_t InflationMatrixSystem::expandRulebook(MomentRulebook &rulebook, const size_t from_symbol) {
        // Debug check rulebook is compatible
        assert(&rulebook.symbols == &(this->Symbols()));
        assert(&rulebook.context == &this->inflationContext);
        assert(&rulebook.factory == &this->polynomial_factory());

        // Do not expand rulebook if it does not permit it, or if it is empty
        if (!rulebook.enable_expansion() || rulebook.empty()) {
            return 0;
        }

        const ptrdiff_t initial_rule_count = rulebook.size();
        const auto& poly_factory = rulebook.factory;

        std::vector<Polynomial> new_rules;

        // Go through factorized symbols, starting from new addition
        for (auto iter = this->factors->begin() + from_symbol;
             iter != this->factors->end(); ++iter) {
            const auto& symbol = *iter;
            // Skip if not factorized (basic substitutions should be already handled)
            if (symbol.fundamental() || symbol.canonical.symbols.empty()) {
                continue;
            }

            const size_t symbol_length = symbol.canonical.symbols.size();
            assert (symbol_length >= 2);

            // Match rules against factors

            std::vector<decltype(rulebook.begin())> match_iterators;
            std::transform(symbol.canonical.symbols.begin(), symbol.canonical.symbols.end(),
                           std::back_inserter(match_iterators),
                           [&](symbol_name_t factor_id) {
                               return rulebook.find(factor_id);
                           });
            assert(match_iterators.size() == symbol_length);

            // If no rules match, go to next factor
            if (std::none_of(match_iterators.begin(), match_iterators.end(),
                             [&](const auto& iter) { return iter != rulebook.end();})) {
                continue;
            }

            // Multiply together all substitutions
            auto get_as_poly = [&](size_t index) {
                if (match_iterators[index] != rulebook.end()) {
                    return match_iterators[index]->second.RHS();
                } else {
                    return Polynomial{Monomial{symbol.canonical.symbols[index], 1.0, false}};
                }
            };
            Polynomial product = get_as_poly(0);
            for (size_t idx=1; idx < symbol_length; ++idx) {
                if (product.empty()) {
                    break;
                }
                product = this->factors->try_multiply(poly_factory, product, get_as_poly(idx));
            }

            poly_factory.append(product, {Monomial{symbol.id, -1.0, false}});

            // Check if this infers anything new?
            new_rules.emplace_back(std::move(product));

        }

        rulebook.add_raw_rules(std::move(new_rules));

        // Compile rules
        ptrdiff_t final_rule_count = rulebook.complete();

        // Turn off rulebook's expansion mode
        rulebook.disable_expansion();

        // How many rules?
        return final_rule_count - initial_rule_count;
    }

}