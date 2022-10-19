/**
 * monomial_substitution_rule.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "integer_types.h"

#include "symbolic/symbol_expression.h"
#include "raw_sequence.h"

#include <iosfwd>
#include <set>
#include <stdexcept>
#include <vector>

namespace NPATK {

    namespace errors {
        class bad_hint : public std::logic_error {
        public:
            bad_hint();
        };

        class invalid_rule : public std::logic_error {
        public:
            explicit invalid_rule(const std::string& why);
        };

    }

    class RawSequence;
    class RawSequenceBook;
    class RuleBook;

    class MonomialSubstitutionRule {
    public:
        using iter_t = std::vector<oper_name_t>::iterator;
        using const_iter_t = std::vector<oper_name_t>::const_iterator;

    private:
        const HashedSequence rawLHS;
        const HashedSequence rawRHS;
        bool negated = false;

    public:
        /** The amount the string-length changes by, on a successful match */
        const ptrdiff_t Delta = 0;

    public:
        MonomialSubstitutionRule(HashedSequence lhs, HashedSequence rhs,
                                 bool negated = false);

        [[nodiscard]] const HashedSequence& LHS() const noexcept { return this->rawLHS; }

        [[nodiscard]] const HashedSequence& RHS() const noexcept { return this->rawRHS; }

        [[nodiscard]] inline const_iter_t matches_anywhere(const_iter_t iter, const_iter_t iter_end) const noexcept {
            return this->rawLHS.matches_anywhere(iter, iter_end);
        }

        [[nodiscard]] std::vector<oper_name_t>
        apply_match_with_hint(const std::vector<oper_name_t>& input, const_iter_t hint) const;

        size_t all_matches(std::vector<SymbolPair>& output, const RawSequenceBook& rsb, const RawSequence& input) const;

        friend std::ostream& operator<<(std::ostream& os, const MonomialSubstitutionRule& msr);

        friend class RuleBook;
    };
}