/**
 * algebraic_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once

#include "../context.h"
#include "raw_sequence_book.h"
#include "monomial_substitution_rule.h"

#include <set>

namespace NPATK {

    namespace errors {
        class bad_substitution : public std::logic_error {
        public:
            std::vector<oper_name_t> sequence;
            bad_substitution(const std::string& what,
                             std::vector<oper_name_t> seq) : std::logic_error(what), sequence{std::move(seq)} { }
        };
    }

    class AlgebraicMatrixSystem;

    class AlgebraicContext : public Context {
    private:
        RawSequenceBook rawSequences;

        std::vector<MonomialSubstitutionRule> monomialRules;

    public:
        explicit AlgebraicContext(size_t operator_count);

        explicit AlgebraicContext(size_t operator_count, std::vector<MonomialSubstitutionRule> rules);

        bool generate_aliases(size_t max_length);

    private:
        [[nodiscard]] std::set<symbol_name_t> one_substitution(const RawSequence& input_sequence) const;
    };
}