/**
 * moment_substitution_rulebook.cpp
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#include "moment_substitution_rulebook.h"

#include "full_combo_ordering.h"
#include "symbol_table.h"

#include <algorithm>
#include <stdexcept>
#include <ranges>

namespace Moment {
    MomentSubstitutionRulebook::MomentSubstitutionRulebook(const SymbolTable &symbolTable,
                                                           std::unique_ptr<SymbolComboFactory> factoryPtr)
        : symbols{symbolTable}, factory{std::move(factoryPtr)}  {
        // Cannot provide null-ptr to factory.
        assert(factory);

        // Symbol table must match factory's symbol table.
        assert(&symbolTable == &factory->symbols);

    }

    void MomentSubstitutionRulebook::add_raw_rules(std::vector<SymbolCombo> &&raw) {
        // Cannot add rules after completion.
        assert(this->rules.empty());

        // Move in rules
        if (this->raw_rules.empty()) {
            this->raw_rules = std::move(raw);
        } else {
            this->raw_rules.reserve(this->raw_rules.size() + raw.size());
            std::move(raw.begin(), raw.end(), std::back_inserter(this->raw_rules));
        }
    }

    bool MomentSubstitutionRulebook::inject(MomentSubstitutionRule &&msr) {
        const auto id = msr.LHS();
        auto [iter, added] = this->rules.insert(std::make_pair(id, std::move(msr)));

        return added;
    }

    size_t MomentSubstitutionRulebook::complete() {
        // Rules already complete:
        if (!this->rules.empty()) {
            return 0;
        }

        // Nothing to do, thus, already complete
        if (this->raw_rules.empty()) {
            return 0;
        }

        // First, sort raw rules by lowest leading monomial, tie-breaking with shorter strings first.
        std::sort(this->raw_rules.begin(), this->raw_rules.end(), FullComboOrdering(*this->factory));

        size_t rules_added = 0;

        // Now, attempt to add in each rule in order
        for (auto& rule : this->raw_rules) {
            // First, reduce polynomial according to known rules
            SymbolCombo reduced_rule = reduce(std::move(rule));

            // Second, orient to get leading term
            MomentSubstitutionRule msr{this->symbols, std::move(reduced_rule)};

            // If rule has been reduced to a trivial expression, do not add.
            if (msr.is_trivial()) {
                continue;
            }

            const symbol_name_t reduced_rule_id = msr.LHS();

            // Can we add directly?
            if (this->rules.empty() || (this->rules.crbegin()->first < reduced_rule_id)) {
                // Otherwise, just directly add rule at end
                this->rules.emplace_hint(this->rules.end(),
                                         std::make_pair(reduced_rule_id, std::move(msr)));
                ++rules_added;
                continue;
            }

            // Otherwise, rule had a collision.
            // We must insert its reduced form out of order, and then re-reduce all subsequent rules.
            auto [update_iter, was_new] = this->rules.emplace(std::make_pair(reduced_rule_id, std::move(msr)));
            assert(was_new);
            ++update_iter;
            while (update_iter != this->rules.end()) {
                auto& prior_rule = *update_iter;
                auto reduce_iter = this->first_matching_rule(prior_rule.second.RHS());
                if (reduce_iter != this->rules.crend()) {
                    // Rule needs replacing
                    update_iter->second = MomentSubstitutionRule(prior_rule.second.LHS(),
                                                                 this->reduce_with_rule_hint(reduce_iter, prior_rule.second.RHS()));
                    assert(update_iter->first == update_iter->second.LHS());
                }
                if (update_iter->second.is_trivial()) {
                    update_iter = this->rules.erase(update_iter);
                } else {
                    ++update_iter;
                }
            }
        }

        // Clear raw-rules
        this->raw_rules.clear();

        // Rules are now complete
        return rules_added;
    }

    std::map<symbol_name_t, MomentSubstitutionRule>::const_reverse_iterator
    MomentSubstitutionRulebook::first_matching_rule(const SymbolCombo &combo) const noexcept {
        for (auto iter = this->rules.crbegin(); iter != this->rules.crend(); ++iter) {
            if (iter->second.matches(combo)) {
                return iter;
            }
        }

        // No match
        return this->rules.crend();
    }


    SymbolCombo MomentSubstitutionRulebook::reduce_with_rule_hint(
            std::map<symbol_name_t, MomentSubstitutionRule>::const_reverse_iterator rule_hint,
            Moment::SymbolCombo polynomial) const {

        // No factory required for move constructor
        SymbolCombo output{std::move(polynomial)};

        // Iterate, starting with supplied hint
        for (; rule_hint != this->rules.crend(); ++rule_hint) {
            const auto& [lhs, rule] = *rule_hint;
            auto [matches, hint] = rule.match_info(output);
            if (matches > 0) {
                output = rule.reduce_with_hint(*this->factory, output, hint, matches == 2);
            }
        }
        return output;
    }



    bool MomentSubstitutionRulebook::collides(const MomentSubstitutionRule &msr) const noexcept {
        auto find_iter = std::find_if(this->rules.begin(), this->rules.cend(),
                                     [&](const auto& old) {
                                            return old.first == msr.LHS();
        });
        return find_iter != this->rules.cend();
    }

    bool MomentSubstitutionRulebook::collides_at_end(const MomentSubstitutionRule &msr) const noexcept {
        // Cannot collide with empty ruleset
        if (this->rules.empty()) {
            return false;
        }
        // Collides at end if last entry key matches this key
        return this->rules.crbegin()->first == msr.LHS();
    }

}