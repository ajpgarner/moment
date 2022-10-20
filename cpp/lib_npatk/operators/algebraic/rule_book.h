/**
 * rule_book.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "monomial_substitution_rule.h"

#include "operators/shortlex_hasher.h"

#include <map>
#include <vector>

namespace NPATK {
    class AlgebraicContext;

    class RuleBook {
    public:
        using rule_map_t = std::map<size_t, MonomialSubstitutionRule>;
    private:
        const ShortlexHasher& hasher;

        rule_map_t monomialRules;

    public:
        RuleBook(const ShortlexHasher& hasher, const std::vector<MonomialSubstitutionRule>& rules);

        explicit RuleBook(const ShortlexHasher& hasher)
            : RuleBook(hasher, std::vector<MonomialSubstitutionRule>{}) { }

        /** Handle to rules */
        [[nodiscard]] const auto& rules() const noexcept { return this->monomialRules; }

        bool simplify_rules(size_t max_iterations);

        /** Reduce sequence, to best of knowledge, using rules */
        [[nodiscard]] HashedSequence reduce(const HashedSequence& input) const;

        /** Reduce rule, to best of knowledge, using rules in set */
        [[nodiscard]] MonomialSubstitutionRule reduce(const MonomialSubstitutionRule& input) const;

        /**
         * Simplify any rules in the set that can be reduced by other rules.
         * @return Number of changed rules.
         */
        size_t reduce_ruleset();

        /**
         * Attempt to deduce a novel and non-trivial rule from considering overlaps within ruleset (Knuth-Bendix).
         * @return True, if a non-trivial rule was found.
         */
        bool try_new_reduction();

    };

}