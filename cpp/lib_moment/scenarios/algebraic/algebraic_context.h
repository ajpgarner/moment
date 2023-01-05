/**
 * algebraic_context.h
 * 
 * Copyright (c) 2022 Austrian Academy of Sciences
 */
#pragma once


#include "integer_types.h"
#include "../context.h"
#include "rule_book.h"
#include "monomial_substitution_rule.h"

#include "symbolic/symbol_expression.h"

#include <memory>
#include <set>
#include <vector>

namespace Moment::Algebraic {

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
        /** Monomial substitution rules */
        RuleBook rules;

    public:
        AlgebraicContext(size_t operator_count, bool self_adjoint, bool commutative,
                                  const std::vector<MonomialSubstitutionRule>& rules);

        explicit AlgebraicContext(size_t operator_count, bool self_adjoint = true, bool commutative = false)
            : AlgebraicContext{operator_count, self_adjoint, commutative, {}} { }

        ~AlgebraicContext() noexcept override;

        /**
         * Attempt to complete rule set
         * @param max_attempts Number of merges allowed before completion is aborted.
         * @param logger Rule logger, for output
         * @return True, if rule-set was completed.
         */
        bool attempt_completion(size_t max_attempts, RuleLogger * logger = nullptr);

        /**
         * Simplify operator sequence using rules
         */
        bool additional_simplification(sequence_storage_t &op_sequence, bool& negated) const override;

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
    };
}