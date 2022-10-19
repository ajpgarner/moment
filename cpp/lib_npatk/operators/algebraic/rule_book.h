/**
 * rule_book.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "monomial_substitution_rule.h"

#include <map>
#include <vector>

namespace NPATK {
    class AlgebraicContext;

    class RuleBook {
    private:
        const ShortlexHasher& hasher;
        std::map<size_t, MonomialSubstitutionRule> monomialRules;

    public:
        RuleBook(const ShortlexHasher& hasher, const std::vector<MonomialSubstitutionRule>& rules);

        explicit RuleBook(const ShortlexHasher& hasher)
            : RuleBook(hasher, std::vector<MonomialSubstitutionRule>{}) { }

        bool simplify_rules(size_t max_iterations);

        bool simplify_once();

    public:
        [[nodiscard]] static std::vector<oper_name_t>
        concat_merge_lhs(const MonomialSubstitutionRule& ruleA, const MonomialSubstitutionRule& ruleB);

        [[nodiscard]] static ptrdiff_t rule_overlap_lhs(const MonomialSubstitutionRule& ruleA,
                                                        const MonomialSubstitutionRule& ruleB) {
            return ruleA.LHS().suffix_prefix_overlap(ruleB.LHS());
        }

        [[nodiscard]] static std::vector<oper_name_t>
        concat_merge_lhs(const MonomialSubstitutionRule& ruleA,
                         const MonomialSubstitutionRule& ruleB,
                         ptrdiff_t overlap);

        [[nodiscard]] MonomialSubstitutionRule combine_rules(const MonomialSubstitutionRule& lhs,
                                                             const MonomialSubstitutionRule& rhs,
                                                             ptrdiff_t overlap) const;

    };

}