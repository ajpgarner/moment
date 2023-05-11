/**
 * algebraic_context.h
 * 
 * @copyright Copyright (c) 2022 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */
#pragma once

#include "integer_types.h"

#include "../context.h"

#include "algebraic_precontext.h"
#include "monomial_substitution_rule.h"
#include "rule_book.h"

#include "symbolic/symbol_expression.h"

#include <memory>
#include <set>
#include <vector>

namespace Moment::Algebraic {

    class NameTable;

    namespace errors {
        class bad_substitution : public std::logic_error {
        public:
            explicit bad_substitution(const std::string& what) : std::logic_error(what) { }
        };
    }

    class AlgebraicContext : public Context {
    public:
        /** True, if all operators are self-adjoint */
        const bool self_adjoint = true;

        /** True, if all operators are commutative */
        const bool commutative = false;

    private:
        /** Additional operator manipulation */
        AlgebraicPrecontext precontext;

        /** Monomial substitution rules */
        RuleBook rules;

        /** Names */
        std::unique_ptr<NameTable> op_names;

    public:
        /**
         * Construct context from pre-context and names.
         * @param apc Pre-context (containing operator count, self-adjointness, conjugate type, etc.)
         * @param names Names of each operator.
         * @param commutative True, to add commutation rules for every operator.
         * @param normal True, to add normality rules for every operator.
         * @param rules Custom re-write rules.
         */
        AlgebraicContext(const AlgebraicPrecontext& apc,
                         std::unique_ptr<NameTable> names,
                         bool commutative, bool normal,
                         const std::vector<MonomialSubstitutionRule>& rules);

        /** Delegates to first constructor, making default name table */
        AlgebraicContext(const AlgebraicPrecontext& apc, bool commutative, bool normal,
                         const std::vector<MonomialSubstitutionRule>& rules);

        /** Delegates to first constructor with default name table and default rules */
        AlgebraicContext(const AlgebraicPrecontext& apc, bool commutative, bool normal)
            : AlgebraicContext{apc, commutative, normal, {}} {}

        /** Delegates to first constructor, with hermitian, non-commutative APC of supplied size. */
        explicit AlgebraicContext(oper_name_t num_ops);

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
         * Summarize the substitution rules as a string.
         */
        [[nodiscard]] std::string resolved_rules() const;

        /**
         * Conjugate the supplied operator sequence; taking into account possible non-Hermitian operators.
         * @param seq The sequence to conjugate.
         * @return The conjugated sequence.
         */
        [[nodiscard]] OperatorSequence conjugate(const OperatorSequence &seq) const final;

        [[nodiscard]]  std::string format_sequence(const OperatorSequence &seq) const override;

        /**
         * Access rule information.
         */
        [[nodiscard]] const RuleBook& rulebook() const noexcept { return this->rules; }

        /**
         * Access name information.
         */
        [[nodiscard]] const NameTable& names() const noexcept { return *this->op_names; }

    public:
        /** Named c'tor. */
        static std::unique_ptr<AlgebraicContext> FromNameList(std::initializer_list<std::string> names);
    };
}