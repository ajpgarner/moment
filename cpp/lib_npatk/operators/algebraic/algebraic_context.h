/**
 * algebraic_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once


#include "integer_types.h"
#include "../context.h"
#include "raw_sequence_book.h"
#include "monomial_substitution_rule.h"

#include "symbolic/symbol_expression.h"

#include <memory>
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

    class SymbolSet;

    class AlgebraicContext : public Context {
    private:
        RawSequenceBook rawSequences;

        std::unique_ptr<SymbolSet> buildSet;

        std::vector<MonomialSubstitutionRule> monomialRules;

    public:
        explicit AlgebraicContext(size_t operator_count);

        explicit AlgebraicContext(size_t operator_count, std::vector<MonomialSubstitutionRule> rules);

        ~AlgebraicContext() noexcept override;

        bool generate_aliases(size_t max_length);

    private:
        size_t one_substitution(std::vector<SymbolPair>& output, const RawSequence& input_sequence) const;
    };
}