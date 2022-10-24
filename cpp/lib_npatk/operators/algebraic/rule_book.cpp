/**
 * rule_book.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "rule_book.h"
#include "algebraic_context.h"

#include <iostream>

namespace NPATK {


    void OStreamRuleLogger::log_rule_reduced(const MonomialSubstitutionRule& old_rule,
                                             const MonomialSubstitutionRule& new_rule) {
        os << "Reduce:\t" << old_rule << "\n  |-\t" << new_rule << "\n";
    }

    void OStreamRuleLogger::log_rule_removed(const MonomialSubstitutionRule& ex_rule) {
        os << "Remove:\t" << ex_rule << "\n";
    }

    void OStreamRuleLogger::log_rule_introduced(const MonomialSubstitutionRule& parent_rule_a,
                                                const MonomialSubstitutionRule& parent_rule_b,
                                                const MonomialSubstitutionRule& new_rule) {
        os << "Combine:\t" << parent_rule_a << "\tand " << parent_rule_b << ":"
           << "\n  |-\t" << new_rule << "\n";
    }

    RuleBook::RuleBook(const ShortlexHasher& hasher, const std::vector<MonomialSubstitutionRule>& rules)
         : hasher{hasher} {
        for (const auto& rule : rules) {
            // Skip trivial rules
            if (!rule.trivial()) {
                this->monomialRules.insert(std::make_pair(rule.rawLHS.hash, rule));
            }
        }

    }

    bool RuleBook::complete(size_t max_iterations, RuleLogger * logger) {
        size_t iteration = 0;
        while(iteration < max_iterations) {
            if (!this->try_new_reduction(logger)) {
                return true;
            }
            ++iteration;
        }

        // Maximum iterations reached: see if we're complete (i.e. did last rule introduced complete us...)
        return this->is_complete();
    }

    HashedSequence RuleBook::reduce(const HashedSequence& input) const {
        // Look through, and apply first match.
        auto rule_iter = this->monomialRules.begin();

        std::vector<oper_name_t> test_sequence(input.begin(), input.end());

        while (rule_iter != this->monomialRules.end()) {
            const auto& rule = rule_iter->second;

            auto match_iter = rule.matches_anywhere(test_sequence.begin(), test_sequence.end());
            if (match_iter != test_sequence.end()) {
                auto replacement_sequence = rule.apply_match_with_hint(test_sequence, match_iter);
                test_sequence.swap(replacement_sequence);
                // Reset rule iterator, as we now have new sequence to process
                rule_iter = this->monomialRules.begin();
                continue;
            }
            ++rule_iter;
        }

        // No further matches of any rules, stop reduction
        return HashedSequence{std::move(test_sequence), this->hasher};
    }

    MonomialSubstitutionRule RuleBook::reduce(const MonomialSubstitutionRule &input) const {
        // Reduce
        HashedSequence lhs = this->reduce(input.rawLHS);
        HashedSequence rhs = this->reduce(input.rawRHS);

        // Orient
        if (lhs.hash > rhs.hash) {
            return MonomialSubstitutionRule{std::move(lhs), std::move(rhs)};
        } else {
            return MonomialSubstitutionRule{std::move(rhs), std::move(lhs)};
        }
    }


    size_t RuleBook::reduce_ruleset(RuleLogger * logger) {
        size_t number_reduced = 0;

        auto rule_iter = this->monomialRules.begin();
        while (rule_iter != this->monomialRules.end()) {
            MonomialSubstitutionRule isolated_rule{std::move(rule_iter->second)};

            // Pop off rule from set...
            rule_iter = this->monomialRules.erase(rule_iter);

            // Do reduction...
            MonomialSubstitutionRule reduced_rule = this->reduce(isolated_rule);

            // By definition, reduction is non-increasing of hash, so we can reinsert "before" iterator
            size_t reduced_hash = reduced_rule.LHS().hash;
            assert(isolated_rule.LHS().hash >= reduced_hash);

            // If reduction makes rule trivial, it is redundant and can be removed from set...
            if (reduced_rule.trivial()) {
                if (logger) {
                    logger->log_rule_removed(isolated_rule);
                }
                ++number_reduced;
                continue;
            }

            // Test if rule has changed
            if ((isolated_rule.LHS().hash != reduced_rule.LHS().hash) ||
                (isolated_rule.RHS().hash != reduced_rule.RHS().hash)) {
                if (logger) {
                    logger->log_rule_reduced(isolated_rule, reduced_rule);
                }
                ++number_reduced;
            }

            // Push reduced rule back into rule-set
            this->monomialRules.insert(std::make_pair(reduced_hash, std::move(reduced_rule)));
        }
        return number_reduced;
    }

    bool RuleBook::is_complete() const {
        // Look for non-trivially overlapping rules
        for (auto iterA = this->monomialRules.begin(); iterA != this->monomialRules.end(); ++iterA) {
            auto &ruleA = iterA->second;
            for (auto iterB = this->monomialRules.begin(); iterB != this->monomialRules.end(); ++iterB) {
                auto &ruleB = iterB->second;
                // Don't compare to self
                if (iterB == iterA) {
                    continue;
                }

                // Can we form a rule by combining?
                auto maybe_combined_rule = ruleA.combine(ruleB, this->hasher);
                if (!maybe_combined_rule.has_value()) {
                    continue;
                }

                // Reduce new rule
                auto combined_reduced_rule = this->reduce(maybe_combined_rule.value());

                // Is the new rule trivial?
                if (combined_reduced_rule.trivial()) {
                    continue;
                }
                // Not complete: a non-trivial rule was found
                return false;
            }
        }
        // Complete: no non-trivial rules were found.
        return true;
    }


    bool RuleBook::try_new_reduction(RuleLogger * logger) {
        // First, reduce
        this->reduce_ruleset(logger);

        // Look for non-trivially overlapping rules
        for (auto iterA = this->monomialRules.begin(); iterA != this->monomialRules.end(); ++iterA) {
            auto& ruleA = iterA->second;
            for (auto iterB = this->monomialRules.begin(); iterB != this->monomialRules.end(); ++iterB) {
                auto& ruleB = iterB->second;
                // Don't compare to self
                if (iterB == iterA) {
                    continue;
                }

                // Can we form a rule by combining?
                auto maybe_combined_rule = ruleA.combine(ruleB, this->hasher);
                if (!maybe_combined_rule.has_value()) {
                    continue;
                }

                // Reduce new rule
                auto combined_reduced_rule = this->reduce(maybe_combined_rule.value());

                // Is the new rule trivial?
                if (combined_reduced_rule.trivial()) {
                    continue;
                }

                // Non-trivial, add it to rule set
                if (logger) {
                    logger->log_rule_introduced(ruleA, ruleB, combined_reduced_rule);
                }
                size_t rule_hash = combined_reduced_rule.LHS().hash;
                this->monomialRules.insert(std::make_pair(rule_hash, std::move(combined_reduced_rule)));

                // Reduce ruleset
                this->reduce_ruleset(logger);

                // Signal a rule was added
                return true;
            }
        }

        return false;
    }

}