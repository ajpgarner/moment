/**
 * moment_substitution_rulebook.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_substitution_rule.h"

#include <map>
#include <memory>
#include <vector>

namespace Moment {
    class Matrix;
    class SymbolTable;

    namespace errors {
        /** Exception thrown when monomial reduction is attempted when rule-set is not monomial state */
        class not_monomial : public std::logic_error {
        public:
            std::string expr;
            std::string result;

            explicit not_monomial(const std::string& exprStr, const std::string& resultStr)
                : std::logic_error(make_err_msg(exprStr, resultStr)), expr{exprStr}, result{resultStr} { }

        private:
            static std::string make_err_msg(const std::string& exprStr, const std::string& resultStr);
        };

    }

    class MomentSubstitutionRulebook {
    public:
        const SymbolTable& symbols;

    private:
        std::map<symbol_name_t, MomentSubstitutionRule> rules;

        std::vector<SymbolCombo> raw_rules;

        std::unique_ptr<SymbolComboFactory> factory;

        bool monomial_rules = true;

        bool hermitian_rules = true;

    public:
        explicit MomentSubstitutionRulebook(const SymbolTable& table)
            : MomentSubstitutionRulebook(table, std::make_unique<SymbolComboFactory>(table)) { }

        explicit MomentSubstitutionRulebook(const SymbolTable& table, std::unique_ptr<SymbolComboFactory> factory);

        /**
         * Add substitution rules in the form of polynomials equal to zero.
         * Completion is deferred until complete() is called.
         */
        void add_raw_rules(std::vector<SymbolCombo>&& raw);

        /**
         * Add substitution rule in the form of polynomial equal to zero.
         * Completion is deferred until complete() is called.
         */
        void add_raw_rule(SymbolCombo&& raw);

        /**
         * Try to add an oriented rule directly.
         * @returns True if rule added, false if collision.
         */
        bool inject(MomentSubstitutionRule&& msr);

        /**
         * Construct an an oriented rule and add it
         */
        template<typename... Args>
        bool inject(Args&&... args) {
            return this->inject(MomentSubstitutionRule(std::forward<Args>(args)...));
        }

        /**
         * Process raw-rules into completed rule-set.
         * @return Number of rules added.
         */
        size_t complete();

        /**
         * Returns iterator to first rule that would reduce sequence.
         */
        [[nodiscard]] std::map<symbol_name_t, MomentSubstitutionRule>::const_reverse_iterator
        first_matching_rule(const SymbolCombo& combo) const noexcept;

        /**
         * Start reduction from hint onwards (will not apply any reductions before hint).
         */
        [[nodiscard]] SymbolCombo reduce_with_rule_hint(
                std::map<symbol_name_t, MomentSubstitutionRule>::const_reverse_iterator rule_hint,
                SymbolCombo combo) const;

        /**
         * Apply all known rules to SymbolCombo.
         */
        [[nodiscard]] SymbolCombo reduce(SymbolCombo combo) const {
            return this->reduce_with_rule_hint(this->rules.crbegin(), std::move(combo));
        }

        /**
         * Apply all known rules to SymbolExpression.
         */
        [[nodiscard]] SymbolCombo reduce(SymbolExpression expr) const;

        /**
         * Apply all known rules to SymbolExpression.
         * @throws
         */
        [[nodiscard]] SymbolExpression reduce_monomial(SymbolExpression expr) const;

        /**
         * Apply reduction to every element of matrix
         * @param symbols Write-access to symbol table.
         * @param matrix
         * @return Newly created matrix, either of type MonomialSubstitutionMatrix or PolynomialSubstitutionMatrix.
         */
        [[nodiscard]] std::unique_ptr<Matrix> reduce(SymbolTable& symbols, const Matrix& matrix) const;

        /**
         * True if rulebook is guaranteed to produce a monomial matrix if it acts on a monomial matrix.
         * False if there exists a rule that will turn monomial matrices into polynomial matrices.
         */
        [[nodiscard]] bool is_monomial() const noexcept { return this->monomial_rules; }

        /**
         * True if rulebook is guaranteed to transform Hermitian matrices into Hermitian matrices.
         * False if there exists a rule that transforms Hermitian symbols into a non-Hermitian SymbolCombo.
         */
        [[nodiscard]] bool is_hermitian() const noexcept { return this->hermitian_rules; }

        /**
         * True if supplied rule matches key already in rulebook.
         * (Complexity O(log(N)) where N are the number of rules.)
         */
        [[nodiscard]] bool collides(const MomentSubstitutionRule& msr) const noexcept;


        /**
         * True if supplied rule matches key at end of rulebook.
         * (Complexity O(1))
         */
        [[nodiscard]] bool collides_at_end(const MomentSubstitutionRule& msr) const noexcept;

        /**
         * True if no reduction rules.
         */
        [[nodiscard]] bool empty() const noexcept { return this->rules.empty(); }

        /**
         * Get number of reduction rules.
         */
        [[nodiscard]] size_t size() const noexcept { return this->rules.size(); }

        /**
         * Begin iteration over rules.
         */
        [[nodiscard]] auto begin() const noexcept { return this->rules.cbegin(); }

        /**
         * End iteration over rules.
         */
        [[nodiscard]] auto end() const noexcept { return this->rules.cend(); }

        /**
         * Return reference to associated SymbolComboFactory.
         */
        [[nodiscard]] const SymbolComboFactory& Factory() const noexcept { return *this->factory; }



    };
}