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
    class MatrixSystem;
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
        using raw_map_t = std::map<symbol_name_t, double>;

        using raw_complex_map_t = std::map<symbol_name_t, std::complex<double>>;

        const SymbolTable& symbols;


    private:
        std::map<symbol_name_t, MomentSubstitutionRule> rules;

        std::vector<Polynomial> raw_rules;

        std::unique_ptr<PolynomialFactory> factory;

        bool monomial_rules = true;

        bool hermitian_rules = true;

    public:
        explicit MomentSubstitutionRulebook(const SymbolTable& table)
            : MomentSubstitutionRulebook(table, std::make_unique<PolynomialFactory>(table)) { }

        explicit MomentSubstitutionRulebook(const SymbolTable& table, std::unique_ptr<PolynomialFactory> factory);

        /**
         * Add substitution rules in the form of polynomials equal to zero.
         * Completion is deferred until complete() is called.
         */
        void add_raw_rules(std::vector<Polynomial>&& raw);

        /**
         * Add substitution rules in the form of symbol equal to value map.
         * Completion is deferred until complete() is called.
         */
        void add_raw_rules(const raw_map_t& raw);

        /**
         * Add substitution rules in the form of symbol equal to value map.
         * Completion is deferred until complete() is called.
         */
        void add_raw_rules(const raw_complex_map_t& raw);

        /**
         * Add substitution rule in the form of polynomial equal to zero.
         * Completion is deferred until complete() is called.
         */
        void add_raw_rule(Polynomial&& raw);

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
         * Attempt to infer additional rules from factorization structure
         * @param A matrix system with associated factor table.
         * @return The number of new rules inferred.
         */
        size_t infer_additional_rules_from_factors(const MatrixSystem& ms);

        /**
         * Apply all known rules to Polynomial.
         * @return true if rules were applied.
         */
        bool reduce_in_place(Polynomial& combo) const;

        /**
         * Apply all known rules to Polynomial.
         */
        [[nodiscard]] Polynomial reduce(Polynomial combo) const {
            this->reduce_in_place(combo);
            return combo;
        }

        /**
         * Apply all known rules to Monomial.
         */
        [[nodiscard]] Polynomial reduce(Monomial expr) const;

        /**
         * Apply all known rules to Monomial.
         * @throws
         */
        [[nodiscard]] Monomial reduce_monomial(Monomial expr) const;

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
         * False if there exists a rule that transforms Hermitian symbols into a non-Hermitian Polynomial.
         */
        [[nodiscard]] bool is_hermitian() const noexcept { return this->hermitian_rules; }

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
         * True if there are pending rules to complete
         */
        [[nodiscard]] bool pending_rules() const noexcept { return !this->raw_rules.empty(); }

        /**
         * Return reference to associated PolynomialFactory.
         */
        [[nodiscard]] const PolynomialFactory& Factory() const noexcept { return *this->factory; }
    };
}