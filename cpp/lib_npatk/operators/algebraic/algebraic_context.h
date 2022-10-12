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
    public:
        /** True, if all operators are self-adjoint */
        const bool self_adjoint = true;

    private:
        /** Collection of every permutation of symbols */
        RawSequenceBook rawSequences;

        /** The set of substitutions */
        std::unique_ptr<SymbolSet> buildSet;

        /** Monomial substitution rules */
        std::vector<MonomialSubstitutionRule> monomialRules;

        /** Calculated substitutions [key: hash, value: index of replacement sequence in raw sequences] */
        std::map<size_t, size_t> hashToReplacementSymbol;

    public:
        explicit AlgebraicContext(size_t operator_count, bool self_adjoint = true);

        explicit AlgebraicContext(size_t operator_count, bool self_adjoint,
                                  std::vector<MonomialSubstitutionRule> rules);

        ~AlgebraicContext() noexcept override;

        bool generate_aliases(size_t max_length);

        bool additional_simplification(std::vector<oper_name_t>& op_sequence) const override;

        /**
         * Summarize the context as a string.
         */
        [[nodiscard]] std::string to_string() const override;


    private:
        size_t one_substitution(std::vector<SymbolPair>& output, const RawSequence& input_sequence) const;
    };
}