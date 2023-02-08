/**
 * rule_book.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "rule_book.h"
#include "algebraic_context.h"

#include <iostream>

namespace Moment::Algebraic {


    RuleBook::RuleBook(const AlgebraicPrecontext& pc,
                       const std::vector<MonomialSubstitutionRule>& rules,
                       const bool is_herm) :
            precontext{pc}, is_hermitian{is_herm} {
        this->add_rules(rules);
    }

    ptrdiff_t RuleBook::add_rules(const std::vector<MonomialSubstitutionRule> &rules, RuleLogger * logger) {
        ptrdiff_t added = 0;
        for (const auto& rule : rules) {
            added += this->add_rule(rule, logger);
        }
        return added;
    }



    ptrdiff_t RuleBook::add_rule(const MonomialSubstitutionRule& rule, RuleLogger * logger) {
        // Skip trivial rule
        if (rule.trivial()) {
            return 0;
        }

        // Check if rule LHS already exists
        auto hashLHS = rule.rawLHS.hash();
        auto preexistingIter = this->monomialRules.find(hashLHS);
        if (preexistingIter == this->monomialRules.end()) {

            // LHS doesn't already exist, just insert directly (making sure no 'minus zero' targets)
            if (rule.RHS().zero() && rule.negated()) {
                const auto& [inSituRule, newElem] = this->monomialRules.insert(std::make_pair(hashLHS,
                                                          MonomialSubstitutionRule{rule.LHS(), rule.RHS()}
                                                          ));
                if (nullptr != logger) {
                    logger->rule_introduced(inSituRule->second);
                }

            } else {
                this->monomialRules.insert(std::make_pair(hashLHS, rule));
                if (nullptr != logger) {
                    logger->rule_introduced(rule);
                }
            }

            return 1;
        }

        // Rule already exists...
        auto &preexisting = preexistingIter->second;
        if (preexisting.rawRHS == rule.rawRHS) {
            // Is rule completely redundant?
            if (rule.negated() == preexisting.negated()) {
                return 0;
            }

            // One rule is positive, the other is negative: hence zero is implied...
            if (rule.negated() != preexisting.negated()) {
                ptrdiff_t rules_added = 0;
                MonomialSubstitutionRule lhsToZero{rule.LHS(), HashedSequence(true)};

                // Log reduction of rule LHS...
                if (logger != nullptr) {
                    logger->rule_reduced(preexisting, lhsToZero);
                }
                this->monomialRules.erase(preexistingIter);

                // LHS equal to zero, since we have erased this key we can just add directly...
                this->monomialRules.insert(std::make_pair(hashLHS,
                                                          MonomialSubstitutionRule{rule.LHS(), HashedSequence(true)}
                ));

                // RHS also equal to zero, but we have to add carefully...
                MonomialSubstitutionRule rhsToZero{rule.RHS(), HashedSequence(true)};
                rules_added += this->add_rule(rhsToZero);
                return rules_added;
            }
        }

        // If existing rule (C->A) already majorizes new rule (C->B):
        ptrdiff_t rules_added = 0;
        const bool negated = rule.negated() != preexisting.negated();
        if (preexisting.RHS().hash() < rule.RHS().hash()) {
            // Then new rule should be B->A:
            MonomialSubstitutionRule b_to_a{rule.RHS(), preexisting.RHS(), negated};
            rules_added += this->add_rule(b_to_a); // Add carefully, in case 'B' already exists...
        } else {
            // Otherwise, existing rule C->B is majorized by new rule C->A:
            // We will need to prepare a new rule B->A
            MonomialSubstitutionRule b_to_a{preexisting.RHS(), rule.RHS(), negated};

            // Log removal of rule...
            if (logger != nullptr) {
                logger->rule_removed(preexisting);
                logger->rule_introduced(rule);
            }
            // Remove existing rule
            this->monomialRules.erase(preexistingIter);
            // Since we have erased this key we can just add directly C->A
            this->monomialRules.insert(std::make_pair(hashLHS, rule));

            // Add B->A carefully
            rules_added += this->add_rule(b_to_a);
        }
        return rules_added;
    }

    bool RuleBook::complete(size_t max_iterations, RuleLogger * logger) {
        const bool mock_mode = max_iterations == 0;

        // First, if we are a Hermitian ruleset, introduce initial conjugate rules
        size_t iteration = 0;

        if (this->is_hermitian) {
            size_t new_rules = this->conjugate_ruleset(mock_mode, logger);
            // If no iterations allowed, but conjugation introduces non-trivial rules, then flag as incomplete
            if (mock_mode && new_rules > 0) {
                return false;
            }
            iteration += new_rules;
        }

        // Now, standard Knuth-Bendix loop
        while(iteration < max_iterations) {
            if (!this->try_new_combination(logger)) {
                if (logger) {
                    logger->success(*this, iteration);
                }
                return true;
            }
            ++iteration;
        }

        // Maximum iterations reached: see if we're complete (i.e. did final rule introduced complete the set?)
        bool is_complete = this->is_complete();
        if (logger) {
            if (is_complete) {
                logger->success(*this, iteration);
            } else {
                logger->failure(*this, iteration);
            }
        }
        return is_complete;
    }

    std::pair<HashedSequence, bool> RuleBook::reduce(const HashedSequence& input) const {
        // Look through, and apply first match.
        auto rule_iter = this->monomialRules.begin();

        bool negated = false;
        sequence_storage_t test_sequence(input.begin(), input.end());

        while (rule_iter != this->monomialRules.end()) {
            const auto& rule = rule_iter->second;

            auto match_iter = rule.matches_anywhere(test_sequence.begin(), test_sequence.end());
            if (match_iter != test_sequence.end()) {
                // Reduced to zero?
                if (rule.rawRHS.zero()) {
                    return {HashedSequence{true}, false};
                }

                // What about negation?
                if (rule.negated()) {
                    negated = !negated;
                }
                auto replacement_sequence = rule.apply_match_with_hint(test_sequence, match_iter);
                test_sequence.swap(replacement_sequence);
                // Reset rule iterator, as we now have new sequence to process
                rule_iter = this->monomialRules.begin();
                continue;
            }
            ++rule_iter;
        }

        // No further matches of any rules, stop reduction
        return {HashedSequence{std::move(test_sequence), this->precontext.hasher}, negated};
    }

    MonomialSubstitutionRule RuleBook::reduce(const MonomialSubstitutionRule &input) const {
        // Reduce
        auto [lhs, lhsNeg] = this->reduce(input.rawLHS);
        auto [rhs, rhsNeg] = this->reduce(input.rawRHS);

        bool negative = input.negated() != (lhsNeg != rhsNeg);

        // Special reduction if rule implies something is zero:
        if ((lhs.hash() == rhs.hash()) && negative) {
            return MonomialSubstitutionRule{std::move(lhs), HashedSequence(true)};
        }

        // Otherwise, orient and return
        if (lhs.hash() > rhs.hash()) {
            return MonomialSubstitutionRule{std::move(lhs), std::move(rhs), negative};
        } else {
            return MonomialSubstitutionRule{std::move(rhs), std::move(lhs), negative};
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
            size_t reduced_hash = reduced_rule.LHS().hash();
            assert(isolated_rule.LHS().hash() >= reduced_hash);

            // If reduction makes rule trivial, it is redundant and can be removed from set...
            if (reduced_rule.trivial()) {
                if (logger) {
                    logger->rule_removed(isolated_rule);
                }
                ++number_reduced;
                continue;
            }

            // Test if rule has changed
            if ((isolated_rule.LHS().hash() != reduced_rule.LHS().hash()) ||
                (isolated_rule.RHS().hash() != reduced_rule.RHS().hash())) {
                if (logger) {
                    logger->rule_reduced(isolated_rule, reduced_rule);
                }
                ++number_reduced;
            }

            // Push reduced rule back into rule-set
            auto [entry_iter, new_entry] = this->monomialRules.insert(std::make_pair(reduced_hash,
                                                                                     std::move(reduced_rule)));
            assert(new_entry); // True, because if hash matches, rule would have been further reduced, or trivial.
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
                auto maybe_combined_rule = ruleA.combine(ruleB, this->precontext);
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


    bool RuleBook::try_new_combination(RuleLogger * logger) {
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
                auto maybe_combined_rule = ruleA.combine(ruleB, this->precontext);
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
                    logger->rule_introduced(ruleA, ruleB, combined_reduced_rule);
                }
                size_t rule_hash = combined_reduced_rule.LHS().hash();
                this->monomialRules.insert(std::make_pair(rule_hash, std::move(combined_reduced_rule)));

                // Reduce ruleset
                this->reduce_ruleset(logger);

                // Signal a rule was added
                return true;
            }
        }

        return false;
    }


    size_t RuleBook::conjugate_ruleset(bool mock, RuleLogger * logger) {
        size_t added = 0;

        auto rule_iter = this->rules().begin();

        while (rule_iter != this->rules().end()) {
            if (this->try_conjugation(rule_iter->second, mock, logger)) {
                // A new rule could be added, so mock mode early return...
                if (mock) {
                    return 1;
                }

                // A new rule was added, iter is de facto invalidated, so restart at beginning of set...
                rule_iter = this->rules().begin();
                ++added;
            }
            // No new rule, try next rule in set...
            ++rule_iter;
        }

        return added;
    }

    bool RuleBook::try_conjugation(const MonomialSubstitutionRule& rule, bool mock, RuleLogger * logger) {
        assert(this->is_hermitian);

        // Conjugate and reduce rule
        auto conj_rule = rule.conjugate(this->precontext);
        auto conj_reduced_rule = this->reduce(conj_rule);

        // Reject rule if it doesn't imply anything new
        if (conj_reduced_rule.trivial()) {
            return false;
        }

        // Otherwise, add reduced rule to set
        if (logger) {
            logger->rule_introduced_conjugate(rule, conj_reduced_rule);
        }

        // Early exit if mock mode
        if (mock) {
            return true;
        }

        size_t rule_hash = conj_reduced_rule.LHS().hash();
        this->monomialRules.insert(std::make_pair(rule_hash, std::move(conj_reduced_rule)));

        // Reduce ruleset
        this->reduce_ruleset(logger);

        return true;
    }

    void RuleBook::commutator_rules(const AlgebraicPrecontext& apc, std::vector<MonomialSubstitutionRule>& output) {
        const oper_name_t operator_count = apc.num_operators;

        // Do nothing, if less than two operators
        if (operator_count < 2) {
            return;
        }

        const auto expected_new_rules = (operator_count*(operator_count-1)) / 2;
        output.reserve(output.size() + expected_new_rules);
        for (oper_name_t b = operator_count-1; b >= 1; --b) {
            for (oper_name_t a = b-1; a >= 0; --a) {
                output.emplace_back(HashedSequence{{b, a}, apc.hasher}, HashedSequence{{a, b}, apc.hasher});
            }
        }
    }


   void RuleBook::normal_rules(const AlgebraicPrecontext& apc, std::vector<MonomialSubstitutionRule>& output) {
        if (apc.self_adjoint || (apc.num_operators == 0)) {
            return;
        }

        const auto raw_operator_count = static_cast<oper_name_t>(apc.num_operators / 2);

        output.reserve(output.size() + raw_operator_count);
        for (oper_name_t a = 0; a < raw_operator_count; ++a) {
            const auto aStar = static_cast<oper_name_t>(a + raw_operator_count);
            output.emplace_back(HashedSequence{{aStar, a}, apc.hasher}, HashedSequence{{a, aStar}, apc.hasher});
        }
    }


    std::ostream &operator<<(std::ostream &os, const RuleBook &rulebook) {
        if (rulebook.is_hermitian) {
            os << "Hermitian rule ";
        } else {
            os << "Rule ";
        }
        os << "book with " << rulebook.size() << ((rulebook.size() != 1) ? " rules" : " rule") << ":\n";

        size_t rule_index = 1;
        for (const auto& [hash, rule] : rulebook.rules()) {
            os << "#" << rule_index << ":\t" << rule << "\n";
            ++rule_index;
        }
        os << "\n";

        return os;
    }

}