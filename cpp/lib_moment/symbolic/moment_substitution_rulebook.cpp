/**
 * moment_substitution_rulebook.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rulebook.h"

#include "polynomial_ordering.h"
#include "symbol_table.h"

#include "matrix/substituted_matrix.h"

#include "scenarios/inflation/inflation_matrix_system.h"
#include "scenarios/inflation/factor_table.h"


#include <algorithm>
#include <stdexcept>
#include <sstream>

namespace Moment {
    namespace errors {
        std::string not_monomial::make_err_msg(const std::string& exprStr, const std::string& resultStr) {
            std::stringstream errSS;
            errSS << "Could not reduce expression \"" << exprStr
                  << "\" as result \"" << resultStr << "\" was not monomial.";
            return errSS.str();
        }
    }

    MomentSubstitutionRulebook::MomentSubstitutionRulebook(const SymbolTable &symbolTable,
                                                           std::unique_ptr<PolynomialFactory> factoryPtr)
        : symbols{symbolTable}, factory{std::move(factoryPtr)}  {
        // Cannot provide null-ptr to factory.
        assert(factory);

        // Symbol table must match factory's symbol table.
        assert(&symbolTable == &factory->symbols);

    }

    void MomentSubstitutionRulebook::add_raw_rules(std::vector<Polynomial> &&raw) {
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        // Move in rules
        if (this->raw_rules.empty()) {
            this->raw_rules = std::move(raw);
        } else {
            this->raw_rules.reserve(this->raw_rules.size() + raw.size());
            std::move(raw.begin(), raw.end(), std::back_inserter(this->raw_rules));
        }
    }


    void MomentSubstitutionRulebook::add_raw_rules(const MomentSubstitutionRulebook::raw_map_t& raw) {
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        this->raw_rules.reserve(this->raw_rules.size() + raw.size());
        for (auto [id, value] : raw) {
            if (approximately_zero(value)) {
                this->raw_rules.emplace_back(Monomial{id});
            } else {
                this->raw_rules.emplace_back(Polynomial{Monomial{id, 1.0}, Monomial{1, -value}});
            }
        }
    }

    void MomentSubstitutionRulebook::add_raw_rules(const MomentSubstitutionRulebook::raw_complex_map_t& raw) {
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        this->raw_rules.reserve(this->raw_rules.size() + raw.size());
        for (auto [id, value] : raw) {
            if (approximately_zero(value)) {
                this->raw_rules.emplace_back(Monomial{id});
            } else {
                this->raw_rules.emplace_back(Polynomial{Monomial{id, 1.0}, Monomial{1, -value}});
            }
        }
    }


    void MomentSubstitutionRulebook::add_raw_rule(Polynomial&& raw) {
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        this->raw_rules.emplace_back(std::move(raw));
    }

    bool MomentSubstitutionRulebook::inject(MomentSubstitutionRule &&msr) {
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        const auto id = msr.LHS();
        assert (id < this->symbols.size());

        // If non-monomial rule is directly injected, rulebook becomes non-monomial.
        if (!msr.RHS().is_monomial()) {
            this->monomial_rules = false;
        }

        // If rule maps Hermitian symbol to non-Hermitian combo, rulebook becomes non-Hermitian.
        if (this->symbols[msr.LHS()].is_hermitian()) {
            if (!msr.RHS().is_hermitian(this->symbols)) {
                this->hermitian_rules = false;
            }
        }

        // Do add
        auto [iter, added] = this->rules.insert(std::make_pair(id, std::move(msr)));
        return added;
    }

    size_t MomentSubstitutionRulebook::complete() {
        // Can't complete if already in use
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        // Nothing to do, thus, already complete
        if (this->raw_rules.empty()) {
            return 0;
        }

        // Rules already complete?
        const bool existing_rules = !this->rules.empty();

        // First, sort raw rules by lowest leading monomial, tie-breaking with shorter strings first.
        std::sort(this->raw_rules.begin(), this->raw_rules.end(), PolynomialOrdering(*this->factory));

        size_t rules_attempted = 0;
        size_t rules_added = 0;
        size_t rules_removed = 0;

        // Now, attempt to add in each rule in order
        for (auto& rule : this->raw_rules) {
            // First, reduce polynomial according to known rules
            Polynomial reduced_rule{this->reduce(std::move(rule))};

            // Second, orient to get leading term
            MomentSubstitutionRule msr{this->symbols, std::move(reduced_rule)};

            // If rule has been reduced to a trivial expression, do not add.
            if (msr.is_trivial()) {
                ++rules_attempted;
                continue;
            }

            const symbol_name_t reduced_rule_id = msr.LHS();

            // Can we add directly to end?
            if (this->rules.empty() || (this->rules.crbegin()->first < reduced_rule_id)) {
                // Otherwise, just directly add rule at end
                this->rules.emplace_hint(this->rules.end(),
                                         std::make_pair(reduced_rule_id, std::move(msr)));
                ++rules_attempted;
                ++rules_added;
                continue;
            }

            // Otherwise, rule had a collision.
            // We must insert its reduced form out of order, and then re-reduce all subsequent rules.
            auto [update_iter, was_new] = this->rules.emplace(std::make_pair(reduced_rule_id, std::move(msr)));
            assert(was_new); // Cannot directly collide, due to reduction.
            ++update_iter;
            while (update_iter != this->rules.end()) {
                auto& prior_rule = *update_iter;
                this->reduce_in_place(prior_rule.second.rhs);
                 ++update_iter;
            }

            ++rules_attempted;
            ++rules_added;
        }

        // Clear raw-rules
        this->raw_rules.clear();

        // Check if completed rule-set is strictly monomial
        this->monomial_rules = std::all_of(this->rules.cbegin(), this->rules.cend(), [](const auto& pair) {
            return pair.second.RHS().is_monomial();
        });


        // Check if completed rule-set is strictly Hermitian
        this->hermitian_rules = std::all_of(this->rules.cbegin(), this->rules.cend(), [&](const auto& pair) {
            if (this->symbols[pair.first].is_hermitian()) {
                return pair.second.RHS().is_hermitian(this->symbols);
            }
            // Rules on non-Hermitian variables can do as they please.
            return true;
        });

        // Rules are now complete
        return rules_added;
    }


    size_t MomentSubstitutionRulebook::combine_and_complete(MomentSubstitutionRulebook &&other) {
        // Can't add new rules if already in use
        if (this->in_use() && other.in_use()) {
            throw errors::already_in_use{};
        }

        // Can't do combination if this has any pending rules
        if (this->pending_rules()) {
            throw std::runtime_error{"Cannot merge rulebook into rulebook which already has rules pending completion."};
        }

        // Can't do combination if other has any pending rules
        if (other.pending_rules()) {
            throw std::runtime_error{"Cannot merge rulebook with rules pending completion into another rulebook."};
        }

        // Special case if this rulebook is empty
        if (this->rules.empty()) {
            this->rules = std::move(other.rules);
            this->monomial_rules = other.monomial_rules;
            this->hermitian_rules = other.hermitian_rules;
            return this->rules.size();
        }

        // Special case if other rulebook is empty
        if (other.rules.empty()) {
            return 0;
        }

        // Translate any processed rules into pending rules
        this->raw_rules.reserve(this->raw_rules.size() + other.rules.size());
        for (auto [id, rule] : other.rules) {
            this->raw_rules.emplace_back(rule.as_polynomial(*this->factory));
        }

        // Finally, do completion in exception-guaranteed manner. Slow, but prevents bad rules from breaking everything.
        std::map<symbol_name_t, MomentSubstitutionRule> old_rules{this->rules};
        try {
            // Attempt completion
            return this->complete();
        } catch(std::exception& e) {
            // In case of fail, restore old rules, and clear the pending list.
            this->rules = std::move(old_rules);
            this->raw_rules.clear();

            // Pass exception forward
            throw;
        }
    }

    size_t MomentSubstitutionRulebook::infer_additional_rules_from_factors(const MatrixSystem &ms) {
        // Can't make new rules if already in use
        if (this->in_use()) {
            throw errors::already_in_use{};
        }

        // If no rules, no additional rules
        if (this->rules.empty()) {
            return 0;
        }

        // Only handle inflation MS here.
        const auto* imsPtr = dynamic_cast<const Inflation::InflationMatrixSystem*>(&ms);
        if (imsPtr == nullptr) {
            return 0;
        }
        const Inflation::InflationMatrixSystem& inflation_system = *imsPtr;
        const auto& factors = inflation_system.Factors();

        std::vector<Polynomial> new_rules;

        // Go through factorized symbols...
        for (const auto& symbol : factors) {
            // Skip if not factorized (basic substitutions should be already handled)
            if (symbol.fundamental() || symbol.canonical.symbols.empty()) {
                continue;
            }

            const size_t symbol_length = symbol.canonical.symbols.size();
            assert (symbol_length >= 2);

            // Match rules against factors
            std::vector<decltype(this->rules.cbegin())> match_iterators;
            std::transform(symbol.canonical.symbols.begin(), symbol.canonical.symbols.end(),
                           std::back_inserter(match_iterators),
                           [&](symbol_name_t factor_id) {
                return this->rules.find(factor_id);
            });
            assert(match_iterators.size() == symbol_length);

            // If no rules match, go to next factor
            if (std::none_of(match_iterators.begin(), match_iterators.end(),
                             [&](const auto& iter) { return iter != this->rules.cend();})) {
                continue;
            }

            // Multiply together all substitutions
            auto get_as_poly = [&](size_t index) {
                if (match_iterators[index] != this->rules.cend()) {
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
                product = factors.try_multiply(*this->factory, product, get_as_poly(idx));
            }

            this->factory->append(product, {Monomial{symbol.id, -1.0, false}});

            // Check if this infers anything new?
            new_rules.emplace_back(std::move(product));

        }

        this->add_raw_rules(std::move(new_rules));

        // Compile rules
        return this->complete();
    }

    bool MomentSubstitutionRulebook::reduce_in_place(Moment::Polynomial& polynomial) const {
        Polynomial::storage_t potential_output;
        bool ever_matched = false;
        for (auto poly_iter = polynomial.begin(); poly_iter != polynomial.end(); ++poly_iter) {
            auto rule_iter = this->rules.find(poly_iter->id);

            // Rule match?
            if (rule_iter != rules.cend()) {
                const auto& rule = rule_iter->second;
                if (!ever_matched) {
                    potential_output.reserve(polynomial.size() + rule_iter->second.RHS().size() - 1);
                    std::copy(polynomial.begin(), poly_iter, std::back_inserter(potential_output));
                    ever_matched = true;
                }
                rule.append_transformed(*poly_iter, std::back_inserter(potential_output));
            } else {
                // No match, but we still need to copy:
                if (ever_matched) {
                    potential_output.emplace_back(*poly_iter);
                }
            };
        }

        // If match made, replace rule
        if (ever_matched) {
            polynomial = (*this->factory)(std::move(potential_output));
        }
        return ever_matched;
    }

    Monomial MomentSubstitutionRulebook::reduce_monomial(Monomial expr) const {
        auto rule_iter = this->rules.find(expr.id);
        // No match, pass through:
        if (rule_iter == this->rules.cend()) {
            return expr;
        }
        // Otherwise, verify rule results in monomial
        const auto& rule = rule_iter->second;
        if (!rule.RHS().is_monomial()) {
            auto wrong_answer = rule.reduce(*this->factory, expr);
            throw errors::not_monomial{expr.as_string(), wrong_answer.as_string()};
        }

        return rule.reduce_monomial(this->symbols, expr);
    }

    Polynomial MomentSubstitutionRulebook::reduce(Monomial expr) const {
        auto rule_iter = this->rules.find(expr.id);
        // No match, pass through (promote to combo)
        if (rule_iter == this->rules.cend()) {
            return Polynomial{expr};
        }

        // Otherwise, make substitution
        return rule_iter->second.reduce(*this->factory, expr);
    }

    std::unique_ptr<Matrix> MomentSubstitutionRulebook::create_substituted_matrix(SymbolTable& wSymbols,
                                                                                  const Matrix &matrix) const {
        assert(&matrix.Symbols == &wSymbols);

        // Once this line is passed, MSR is in use:
        this->usages.fetch_add(1, std::memory_order_release);

        if (matrix.is_polynomial()) {
            const auto& polyMatrix = dynamic_cast<const PolynomialMatrix&>(matrix);
            return std::make_unique<PolynomialSubstitutedMatrix>(wSymbols, *this, polyMatrix);
        } else {
            const auto& monomialMatrix = dynamic_cast<const MonomialMatrix&>(matrix);
            if (this->is_monomial()) {
                return std::make_unique<MonomialSubstitutedMatrix>(wSymbols, *this, monomialMatrix);
            } else {
                return std::make_unique<PolynomialSubstitutedMatrix>(wSymbols, *this, monomialMatrix);
            }
        }
    }
}