/**
 * rule_book.cpp
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#include "rule_book.h"
#include "algebraic_context.h"
#include "utilities/suffix_prefix.h"

namespace NPATK {


    RuleBook::RuleBook(const ShortlexHasher& hasher, const std::vector<MonomialSubstitutionRule>& rules)
         : hasher{hasher} {
        for (const auto& rule : rules) {
            this->monomialRules.insert(std::make_pair(rule.rawLHS.hash, rule));
        }

    }

    bool RuleBook::simplify_rules(size_t max_iterations) {
        if (this->simplify_once()) {
            return true;
        }

        return false;
    }

    bool RuleBook::simplify_once() {
        std::vector<MonomialSubstitutionRule> newRules;

        for (auto iterA = this->monomialRules.begin(); iterA != this->monomialRules.end(); ++iterA) {
            auto& ruleA = iterA->second;
            for (auto iterB = this->monomialRules.begin(); iterB != this->monomialRules.end(); ++iterB) {
                auto& ruleB = iterB->second;
                // Don't compare to self
                if (iterB == iterA) {
                    continue;
                }

                auto overlap_size = rule_overlap_lhs(ruleA, ruleB);
                if (overlap_size > 0) {
                    auto combined_rule = this->combine_rules(ruleA, ruleB, overlap_size);
                    newRules.push_back(std::move(combined_rule));
                }
            }
        }

        return !newRules.empty();
    }

    std::vector<oper_name_t> RuleBook::concat_merge_lhs(const MonomialSubstitutionRule& ruleA,
                                                        const MonomialSubstitutionRule& ruleB)  {
        auto overlap_size = rule_overlap_lhs(ruleA, ruleB);
        return concat_merge_lhs(ruleA, ruleB, overlap_size);
    }



    ptrdiff_t RuleBook::rule_overlap_lhs(const MonomialSubstitutionRule &ruleA, const MonomialSubstitutionRule &ruleB) {
        return static_cast<ptrdiff_t>(suffix_prefix(ruleA.rawLHS.operators, ruleB.rawLHS.operators));
    }

    std::vector<oper_name_t> RuleBook::concat_merge_lhs(const MonomialSubstitutionRule& ruleA,
                                                        const MonomialSubstitutionRule& ruleB,
                                                        ptrdiff_t overlap_size)  {
        std::vector<oper_name_t> joined_string;
        joined_string.reserve(static_cast<ptrdiff_t>(ruleA.rawLHS.operators.size() + ruleB.rawLHS.operators.size())
                              - overlap_size);
        std::copy(ruleA.rawLHS.operators.cbegin(), ruleA.rawLHS.operators.cend() - static_cast<ptrdiff_t>(overlap_size),
                  std::back_inserter(joined_string));
        std::copy(ruleB.rawLHS.operators.cbegin(), ruleB.rawLHS.operators.cend(),
                  std::back_inserter(joined_string));

        return joined_string;
    }


    [[nodsicard]] MonomialSubstitutionRule RuleBook::combine_rules(const MonomialSubstitutionRule& ruleA,
                                                                   const MonomialSubstitutionRule& ruleB,
                                                                   ptrdiff_t overlap_size) const {
        // Get string from joining rules
        auto joined_string = concat_merge_lhs(ruleA, ruleB, overlap_size);

        // Apply rule to joint string
        auto rawViaRuleA = ruleA.apply_match_with_hint(joined_string, joined_string.begin());
        auto rawHashA = this->hasher(rawViaRuleA);

        auto furtherSimpA = this->monomialRules.find(rawHashA);
        if (furtherSimpA != this->monomialRules.cend()) {
            rawViaRuleA = furtherSimpA->second.rawRHS.operators;
            rawHashA =  furtherSimpA->second.rawRHS.hash;
        }

        auto rawViaRuleB = ruleB.apply_match_with_hint(joined_string,
                                                       joined_string.cend() - static_cast<ptrdiff_t>(ruleB.rawLHS.size()));
        auto rawHashB = this->hasher(rawViaRuleB);

        auto furtherSimpB = this->monomialRules.find(rawHashB);
        if (furtherSimpB != this->monomialRules.cend()) {
            rawViaRuleB = furtherSimpB->second.rawRHS.operators;
            rawHashB =  furtherSimpB->second.rawRHS.hash;
        }

        HashedSequence lexViaA{std::move(rawViaRuleA), rawHashA};
        HashedSequence lexViaB{std::move(rawViaRuleB), rawHashB};

        if (lexViaA < lexViaB) {
            return MonomialSubstitutionRule{std::move(lexViaB), std::move(lexViaA)};
        } else {
            return MonomialSubstitutionRule{std::move(lexViaA), std::move(lexViaB)};
        }
    }



}