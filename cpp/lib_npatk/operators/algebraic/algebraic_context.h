/**
 * algebraic_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once


#include "integer_types.h"
#include "../context.h"
#include "raw_sequence_book.h"
#include "rule_book.h"
#include "monomial_substitution_rule.h"

#include "symbolic/symbol_expression.h"

#include <memory>
#include <set>
#include <vector>

namespace NPATK {

    namespace errors {
        class bad_substitution : public std::logic_error {
        public:
            explicit bad_substitution(const std::string& what) : std::logic_error(what) { }
        };
    }

    class SymbolSet;

    class AlgebraicContext : public Context {
    public:
        /** True, if all operators are self-adjoint */
        const bool self_adjoint = true;

        /** True, if all operators are commutative */
        const bool commutative = false;

    private:
        /** Collection of every permutation of symbols */
        RawSequenceBook rawSequences;

        /** Monomial substitution rules */
        RuleBook rules;

        /** The set of substitutions */
        std::unique_ptr<SymbolSet> buildSet;

        /** Calculated substitutions [key: hash, value: index of replacement sequence in raw sequences, negation] */
        std::map<uint64_t, std::pair<size_t, bool>> hashToReplacementSymbol;

    public:
        AlgebraicContext(size_t operator_count, bool self_adjoint, bool commutative,
                                  const std::vector<MonomialSubstitutionRule>& rules);

        explicit AlgebraicContext(size_t operator_count, bool self_adjoint = true, bool commutative = false)
            : AlgebraicContext{operator_count, self_adjoint, commutative, {}} { }

        ~AlgebraicContext() noexcept override;

        bool attempt_completion(size_t max_attempts, RuleLogger * logger = nullptr);

        bool generate_aliases(size_t max_length);

        bool additional_simplification(std::vector<oper_name_t>& op_sequence, bool& negated) const override;

        /**
       * Does context know anything extra known about operator sequence X that would imply Re(X)=0 or Im(X)=0?
       * @param seq The operator sequence X to test.
       * @return Pair, first: true if real part is zero, second: true if imaginary part is zero.
       */
        [[nodiscard]] std::pair<bool, bool> is_sequence_null(const OperatorSequence& seq) const noexcept override;

        /**
         * Summarize the context as a string.
         */
        [[nodiscard]] std::string to_string() const override;

        /**
         * Summarize the substitution rules as a string
         */
        [[nodiscard]] std::string resolved_rules() const;

        /**
         * Access rule information
         */
        [[nodiscard]] const RuleBook& rulebook() const noexcept { return this->rules; }

    private:
        size_t one_substitution(std::vector<SymbolPair>& output, const RawSequence& input_sequence) const;

        void build_hash_table();
    };
}