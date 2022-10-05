/**
 * monomial_substitution_rule.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../operator.h"
#include "symbolic/symbol.h"

#include <set>
#include <stdexcept>
#include <vector>

namespace NPATK {

    namespace errors {
        class bad_hint : public std::logic_error {
        public:
            bad_hint();
        };
    }

    class RawSequence;
    class RawSequenceBook;

    class MonomialSubstitutionRule {
    public:
        using iter_t = std::vector<oper_name_t>::iterator;
        using const_iter_t = std::vector<oper_name_t>::const_iterator;

    private:
        const std::vector<oper_name_t> rawLHS;
        const std::vector<oper_name_t> rawRHS;

    public:
        /** The amount the string-length changes by, on a successful match */
        const ptrdiff_t Delta = 0;

    public:
        constexpr MonomialSubstitutionRule(std::vector<oper_name_t> lhs, std::vector<oper_name_t> rhs)
                : rawLHS(std::move(lhs)), rawRHS(std::move(rhs)),
                  Delta(static_cast<ptrdiff_t>(rawRHS.size()) - static_cast<ptrdiff_t>(rawLHS.size())) {
        }

        [[nodiscard]] bool matches(const_iter_t iter, const_iter_t iter_end) const noexcept;

        [[nodiscard]] const_iter_t matches_anywhere(const_iter_t iter, const_iter_t iter_end) const noexcept;

        [[nodiscard]] std::vector<oper_name_t>
        apply_match_with_hint(const std::vector<oper_name_t>& input, const_iter_t hint) const;

        size_t all_matches(std::set<symbol_name_t>& output, const RawSequenceBook& rsb, const RawSequence& input) const;
    };
}