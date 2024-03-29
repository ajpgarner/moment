/**
 * operator_rulebook.cpp
 * 
 * @copyright Copyright (c) 2022-2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#include "operator_rulebook.h"

#include "utilities/substring_hasher.h"

#include <cmath>

#include <algorithm>
#include <iostream>




namespace Moment::Algebraic {

    OperatorRulebook::OperatorRulebook(const AlgebraicPrecontext& apc,
                                       const std::vector<OperatorRule>& rules) :
            precontext{apc}, is_hermitian{apc.self_adjoint()} {
        this->add_rules(rules);
        this->recalculate_magnitude();
    }

    void OperatorRulebook::recalculate_magnitude() noexcept {
        const size_t count = this->monomialRules.size();
        if (0 == count) {
            this->mag = 0;
        } else if (1 == count) {
            this->mag = 1;
        } else {
            this->mag = static_cast<size_t>(std::ceil(std::log2(static_cast<double>(count))) + 1e-8);
        }
    }

    ptrdiff_t OperatorRulebook::add_rules(const std::span<const OperatorRule> rules, RuleLogger * logger) {
        ptrdiff_t added = 0;
        for (const auto& rule : rules) {
            added += this->do_add_rule(rule, logger);
        }
        if (added != 0) {
            this->recalculate_magnitude();
        }
        return added;
    }

    ptrdiff_t OperatorRulebook::add_rule(const OperatorRule& rule, RuleLogger * logger) {
        ptrdiff_t num_added = do_add_rule(rule, logger);
        if (num_added != 0) {
            this->recalculate_magnitude();
        }
        return num_added;
    }

    ptrdiff_t OperatorRulebook::do_add_rule(const OperatorRule& rule, RuleLogger * logger) {
        // Skip trivial rule
        if (rule.trivial()) {
            return 0;
        }

        // Check if rule LHS already exists
        auto hashLHS = rule.rawLHS.hash();
        auto preexistingIter = this->monomialRules.find(hashLHS);
        if (preexistingIter == this->monomialRules.end()) {

            // LHS doesn't already exist, just insert directly (making sure no 'minus zero' targets)
            if (rule.RHS().zero() && (rule.rule_sign() != SequenceSignType::Positive)) {
                const auto& [inSituRule, newElem] =
                        this->monomialRules.insert(std::make_pair(hashLHS, OperatorRule{rule.LHS(), rule.RHS()}));
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
        if (preexisting.rawRHS.hash() == rule.rawRHS.hash()) {
            // Is rule completely redundant?
            if (rule.rule_sign() == preexisting.rule_sign()) {
                return 0;
            }

            // Rule signs mismatch, and hence zero is implied...
            ptrdiff_t rules_added = 0;
            OperatorRule lhsToZero{rule.LHS(), HashedSequence(true)};

            // Log reduction of rule LHS...
            if (logger != nullptr) {
                logger->rule_reduced(preexisting, lhsToZero);
            }
            this->monomialRules.erase(preexistingIter);

            // LHS equal to zero, since we have erased this key we can just add directly...
            this->monomialRules.insert(std::make_pair(hashLHS,
                                                      OperatorRule{HashedSequence{rule.LHS().raw(),
                                                                                  hashLHS,
                                                                                  SequenceSignType::Positive},
                                                                   HashedSequence(true)}
            ));

            // RHS also equal to zero, but we have to add carefully...
            rules_added += this->do_add_rule(OperatorRule{HashedSequence{rule.RHS().raw(),
                                                                         rule.RHS().hash(),
                                                                         SequenceSignType::Positive},
                                                          HashedSequence(true)}, logger);
            return rules_added;

        }

        // If existing rule (C->A) already majorizes new rule (C->B):
        ptrdiff_t rules_added = 0;

        if (preexisting.RHS().hash() < rule.RHS().hash()) {
            // Then new rule should be B->A:
            OperatorRule b_to_a{rule.RHS(), preexisting.RHS()};
            rules_added += this->do_add_rule(b_to_a, logger); // Add recursively, in case 'B' already exists...
        } else {
            // Otherwise, existing rule C->B is majorized by new rule C->A:
            // We will need to prepare a new rule B->A
            OperatorRule b_to_a{preexisting.RHS(), rule.RHS()};

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
            rules_added += this->do_add_rule(b_to_a, logger);
        }
        return rules_added;
    }

    bool OperatorRulebook::complete(size_t max_iterations, RuleLogger * logger) {
        const bool mock_mode = max_iterations == 0;

        // First, if we are a Hermitian ruleset, introduce initial conjugate rules
        size_t iteration = 0;

        // First, see if any complex conjugate rules are implied
        if (mock_mode) {
            if (this->mock_conjugate()) {
                return false;
            }
        } else {
            iteration += this->conjugate_ruleset( logger);
        }

        // Now, standard Knuth-Bendix loop
        while(iteration < max_iterations) {
            if (!this->try_new_combination(logger)) {
                if (logger) {
                    logger->success(*this, iteration);
                }
                this->recalculate_magnitude();
                return true;
            }
            ++iteration;
        }

        // Maximum iterations reached: see if we're complete (i.e. did final rule introduced complete the set?)
        bool is_complete = this->is_complete(false);
        if (logger) {
            if (is_complete) {
                logger->success(*this, iteration);
            } else {
                logger->failure(*this, iteration);
            }
        }
        this->recalculate_magnitude();
        return is_complete;
    }

    HashedSequence OperatorRulebook::reduce(const HashedSequence& input) const {
        // Empty string, or empty rulebook, should just be forwarded
        if (input.empty()) {
            return HashedSequence{input.zero()};
        } else if (this->monomialRules.empty()) {
            return input;
        }

        // Choose reduction method
        const auto how_to_reduce = this->reduction_method(input.size());

        // Copy (still...)
        sequence_storage_t test_sequence(input.begin(), input.end());
        SequenceSignType sign_type = input.get_sign();
        const auto result = (how_to_reduce == ReductionMethod::SearchRules) ?
                        this->reduce_via_search(test_sequence, sign_type)
                      : this->reduce_via_iteration(test_sequence, sign_type);

        switch (result) {
            case RawReductionResult::NoMatch:
                return input;
            case RawReductionResult::Match:
                return HashedSequence{std::move(test_sequence), this->precontext.hasher, sign_type};
            case RawReductionResult::SetToZero:
                return HashedSequence{true};
            default:
                assert(false);
        }

    }


    OperatorRulebook::RawReductionResult OperatorRulebook::reduce_in_place(HashedSequence& input) const {
        // Empty string, or empty rulebook, no change
        if (input.empty() || this->monomialRules.empty()) {
            return RawReductionResult::NoMatch;
        }

        const auto how_to_reduce = this->reduction_method(input.size());
        SequenceSignType sign_type = input.get_sign();
        const auto result = (how_to_reduce == ReductionMethod::SearchRules) ?
                          this->reduce_via_search(input.raw(), sign_type)
                        : this->reduce_via_iteration(input.raw(), sign_type);

        switch (result) {
            case RawReductionResult::NoMatch:
                break;
            case RawReductionResult::Match:
                input.rehash(this->precontext.hasher);
                input.set_sign(sign_type);
                break;
            case RawReductionResult::SetToZero:
                input.raw().clear();
                input.rehash(0);
                break;
        }
        return result;
    }

    OperatorRulebook::RawReductionResult
    OperatorRulebook::reduce_via_iteration(sequence_storage_t& test_sequence, SequenceSignType& sign_type) const {
        // Look through, and apply first match.
        auto rule_iter = this->monomialRules.begin();

        bool matched_once = false;

        while (rule_iter != this->monomialRules.end()) {
            const auto& rule = rule_iter->second;

            auto match_iter = rule.matches_anywhere(test_sequence.begin(), test_sequence.end());
            if (match_iter != test_sequence.end()) {
                // Reduced to zero?
                if (rule.rawRHS.zero()) {
                    sign_type = SequenceSignType::Positive;
                    return RawReductionResult::SetToZero;
                }

                // What about negation?
                sign_type = sign_type * rule.rule_sign();
                test_sequence = rule.apply_match_with_hint(test_sequence, match_iter);
                // Reset rule iterator, as we now have new sequence to process
                rule_iter = this->monomialRules.begin();
                matched_once = true;
                continue;
            }
            ++rule_iter;
        }

        // No further matches of any rules, stop reduction
        return matched_once ? RawReductionResult::Match : RawReductionResult::NoMatch;
    }

    OperatorRulebook::RawReductionResult
    OperatorRulebook::reduce_via_search(sequence_storage_t& test_sequence, SequenceSignType& sign_type) const {
        bool negated = false;
        bool matching = true;
        bool any_matches = false;
        do {
            SubstringHashRange range{test_sequence, this->precontext.hasher.radix};
            auto iter = range.begin();
            const auto iter_end = range.end();

            while (iter != iter_end) {
                // Do we match this substring?
                if (auto found_rule = this->monomialRules.find(*iter); found_rule != this->monomialRules.end()) {
                    // Reduced to zero?
                    if (found_rule->second.rawRHS.zero()) {
                        return RawReductionResult::SetToZero;
                    }

                    any_matches = true;

                    // Otherwise, do a replacement
                    auto * hint = &test_sequence[iter.index()];
                    auto new_sequence = found_rule->second.apply_match_with_hint(test_sequence, hint);
                    test_sequence = std::move(new_sequence);
                    sign_type = sign_type * found_rule->second.rule_sign();
                    break;
                }
                ++iter;
            }

            if (iter == iter_end) {
                matching = false;
            }
        } while (matching == true);

        // No further matches of any rules, stop reduction
        return any_matches ? RawReductionResult::Match : RawReductionResult::NoMatch;
    }


    HashedSequence OperatorRulebook::reduce_via_iteration(const HashedSequence& input) const {
        if (input.empty()) {
            return input;
        } else if (this->monomialRules.empty()) {
            return input;
        }

        // Copy
        sequence_storage_t test_sequence(input.begin(), input.end());
        SequenceSignType sign_type = input.get_sign();
        auto result = this->reduce_via_iteration(test_sequence, sign_type);

        switch (result) {
            case RawReductionResult::NoMatch:
                return input;
            case RawReductionResult::Match:
                return HashedSequence{std::move(test_sequence), this->precontext.hasher, sign_type};
            case RawReductionResult::SetToZero:
                return HashedSequence{true};
            default:
                assert(false);
        }
    }

    HashedSequence OperatorRulebook::reduce_via_search(const HashedSequence& input) const {
        if (input.empty()) {
            return input;
        } else if (this->monomialRules.empty()) {
            return input;
        }

        // Copy
        sequence_storage_t test_sequence(input.begin(), input.end());
        SequenceSignType sign_type = input.get_sign();
        auto result = this->reduce_via_search(test_sequence, sign_type);
        switch (result) {
            case RawReductionResult::NoMatch:
                return input;
            case RawReductionResult::Match:
                return HashedSequence{std::move(test_sequence), this->precontext.hasher, sign_type};
            case RawReductionResult::SetToZero:
                return HashedSequence{true};
            default:
                assert(false);
        }
    }


    OperatorRule OperatorRulebook::reduce(const OperatorRule& input) const {
        // Reduce
        auto lhs = this->reduce(input.rawLHS);
        auto rhs = this->reduce(input.rawRHS);

        const SequenceSignType relative_sign = difference(lhs.get_sign(), rhs.get_sign());

        // Special reduction if rule implies something is zero:
        if ((lhs.hash() == rhs.hash()) && (relative_sign == SequenceSignType::Negative)) {
            lhs.set_sign(SequenceSignType::Positive);
            return OperatorRule{std::move(lhs), HashedSequence(true)};
        }

        // Otherwise, orient and return
        const auto lhs_hash = lhs.hash();
        const auto rhs_hash = rhs.hash();
        if (lhs_hash > rhs_hash) {
            lhs.set_sign(SequenceSignType::Positive);
            rhs.set_sign(relative_sign);
            return OperatorRule{std::move(lhs), std::move(rhs)};
        } else {
            rhs.set_sign(SequenceSignType::Positive);
            lhs.set_sign(conjugate(relative_sign));
            return OperatorRule{std::move(rhs), std::move(lhs)};
        }
    }

    bool OperatorRulebook::can_reduce(const sequence_storage_t& input) const {
        // Cannot reduce if string or rulebook are empty.
        if (input.empty() || this->monomialRules.empty()) {
            return false;
        }

        // How to test?
        auto test_method = this->reduction_method(input.size());

        if (test_method == ReductionMethod::IterateRules) {
            // Check all rules vs. sequence
            return std::any_of(this->monomialRules.cbegin(), this->monomialRules.cend(),
                               [&input](const auto &key_rule_pair) {
                                   auto match_iter = key_rule_pair.second.matches_anywhere(input.begin(), input.end());
                                   return match_iter != input.end();
                               });
        } else {
            assert(test_method == ReductionMethod::SearchRules);

            SubstringHashRange range{input, this->precontext.hasher.radix};
            return std::any_of(range.begin(), range.end(),
                               [this](const uint64_t hash) { return this->monomialRules.contains(hash); });

        }
    }


    size_t OperatorRulebook::reduce_ruleset(RuleLogger * logger) {
        size_t number_reduced = 0;

        auto rule_iter = this->monomialRules.begin();
        while (rule_iter != this->monomialRules.end()) {
            OperatorRule isolated_rule{std::move(rule_iter->second)};

            // Pop off rule from set...
            rule_iter = this->monomialRules.erase(rule_iter);

            // Do reduction...
            OperatorRule reduced_rule = this->reduce(isolated_rule);

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
                (isolated_rule.RHS() != reduced_rule.RHS())) {
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

    bool OperatorRulebook::is_complete(const bool test_cc) const {
        // Look for CC rules
        if (test_cc && this->mock_conjugate()) {
            return false;
        }

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


    bool OperatorRulebook::try_new_combination(RuleLogger * logger) {
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

    bool OperatorRulebook::mock_conjugate() const {
        for (const auto& [hash, rule] : this->rules()) {
            // Conjugate and reduce rule
            auto conj_rule = rule.conjugate(this->precontext);
            auto conj_reduced_rule = this->reduce(conj_rule);

            // Reject rule if it doesn't imply anything new
            if (conj_reduced_rule.trivial()) {
                continue;
            }

            // Otherwise, we have non-trivial rule
            return true;
        }

        // No non-trivial rules discovered
        return false;
    }

    size_t OperatorRulebook::conjugate_ruleset(RuleLogger * logger) {
        size_t added = 0;

        auto rule_iter = this->rules().begin();

        while (rule_iter != this->rules().end()) {
            if (this->try_conjugation(rule_iter->second, logger)) {

                // A new rule was added, iter is de facto invalidated, so restart at beginning of set...
                rule_iter = this->rules().begin();
                ++added;
            }
            // No new rule, try next rule in set...
            ++rule_iter;
        }

        return added;
    }

    bool OperatorRulebook::try_conjugation(const OperatorRule& rule, RuleLogger * logger) {
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

        size_t rule_hash = conj_reduced_rule.LHS().hash();
        this->monomialRules.insert(std::make_pair(rule_hash, std::move(conj_reduced_rule)));

        // Reduce ruleset
        this->reduce_ruleset(logger);

        return true;
    }

    void OperatorRulebook::commutator_rules(const AlgebraicPrecontext& apc, std::vector<OperatorRule>& output) {
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


   void OperatorRulebook::normal_rules(const AlgebraicPrecontext& apc, std::vector<OperatorRule>& output) {
        if (apc.self_adjoint() || (apc.num_operators == 0)) {
            return;
        }

        const auto raw_operator_count = apc.raw_operators;

        if (apc.conj_mode == AlgebraicPrecontext::ConjugateMode::Bunched) {
            output.reserve(output.size() + raw_operator_count);
            for (oper_name_t a = 0; a < raw_operator_count; ++a) {
                const auto aStar = static_cast<oper_name_t>(a + raw_operator_count);
                output.emplace_back(HashedSequence{{aStar, a}, apc.hasher}, HashedSequence{{a, aStar}, apc.hasher});
            }
        } else {
            assert(apc.conj_mode == AlgebraicPrecontext::ConjugateMode::Interleaved);
            output.reserve(output.size() + raw_operator_count);
            for (oper_name_t idx = 0; idx < raw_operator_count; ++idx) {
                const auto a = static_cast<oper_name_t >(2*idx);
                const auto aStar = static_cast<oper_name_t>(a + 1);
                output.emplace_back(HashedSequence{{aStar, a}, apc.hasher}, HashedSequence{{a, aStar}, apc.hasher});
            }
        }
    }

    std::ostream& operator<<(std::ostream& os, OperatorRulebook::RawReductionResult rrr) {
        switch (rrr) {
            case OperatorRulebook::RawReductionResult::NoMatch:
                os << "NoMatch";
                break;
            case OperatorRulebook::RawReductionResult::Match:
                os << "Match";
                break;
            case OperatorRulebook::RawReductionResult::SetToZero:
                os << "SetToZero";
                break;
        }
        return os;
    }

    std::ostream &operator<<(std::ostream &os, const OperatorRulebook &rulebook) {
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