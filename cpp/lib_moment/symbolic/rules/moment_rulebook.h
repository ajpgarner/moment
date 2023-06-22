/**
 * moment_substitution_rulebook.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_rule.h"

#include <atomic>
#include <map>
#include <memory>
#include <tuple>
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

        /** Exception thrown if rules are added after the ruleset has already been used to reduce matrices */
        class already_in_use : public std::logic_error {
        public:
            already_in_use() : std::logic_error{"No further rules can be added once the rulebook is already in use."}{ }
        };

    }

    class MomentRulebook {
    public:
        /** Result, when checking if one rulebook contains another */
        enum class RulebookComparisonResult {
            /** A is equivalent to B. */
            AEqualsB,
            /** A is a strict superset of B. */
            AContainsB,
            /** B is a strict superset of A. */
            BContainsA,
            /** A and B are strictly disjoint. */
            Disjoint,
        };

    public:
        using raw_map_t = std::map<symbol_name_t, double>;

        using raw_complex_map_t = std::map<symbol_name_t, std::complex<double>>;

        using rule_map_t = std::map<symbol_name_t, MomentRule>;

        using rule_order_map_t = std::map<std::pair<uint64_t, uint64_t>, rule_map_t::iterator>;


        /**
         * Associated symbol table.
         */
        const SymbolTable& symbols;

        /**
         * Associated polynomial factory.
         */
        const PolynomialFactory& factory;

    private:
        std::string human_readable_name;

        /**
         * Not-yet-processed polynomials, that will be subsequently converted into rules.
         */
        std::vector<Polynomial> raw_rules;

        /**
         * Rules, keyed by symbol ID (for quick substitution).
         */
        rule_map_t rules;

        /**
         * Rules, keyed by comparator hash; for iteration in lexicographic order (e.g. while completing).
         */
        rule_order_map_t rules_in_order;

        bool monomial_rules = true;

        bool hermitian_rules = true;

        mutable std::atomic<size_t> usages = 0;

    public:
        explicit MomentRulebook(const MatrixSystem& system);

        explicit MomentRulebook(const MomentRulebook&) = delete;



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
        bool inject(MomentRule&& msr);

        /**
         * Construct an an oriented rule and add it
         */
        template<typename... Args>
        bool inject(Args&&... args) {
            return this->inject(MomentRule(std::forward<Args>(args)...));
        }

        /**
         * Process raw-rules into completed rule-set.
         * @return Number of rules added.
         */
        size_t complete();

        /**
         * Add all rules from another rulebook to this one.
         * Exception guarantee: if merge fails, state of this ruleset is left unchanged.
         * @return Number of rules added.
         */
         size_t combine_and_complete(MomentRulebook&& other);

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
         * Apply all known rules to Polynomial (implicitly creating copy).
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
         * Find first matching rule.
         * @returns Pair: iterator to matching rule, iterator to matching monomial element (or end, end).
         * If one output is not end, other is guaranteed to not be end.
         */
        [[nodiscard]] std::pair<rule_map_t::const_iterator,
                                Polynomial::storage_t::const_iterator> match(const Polynomial& test) const noexcept;
        /**
         * Can only find match in l-values.
         */
        [[nodiscard]] std::pair<rule_map_t::const_iterator,
                                Polynomial::storage_t::const_iterator> match(Polynomial&& test) const = delete;

        /**
         * Apply reduction to every element of matrix, and make a new matrix
         * @param symbols Write-access to symbol table.
         * @param matrix
         * @return Newly created matrix, either of type MonomialSubstitutionMatrix or PolynomialSubstitutionMatrix.
         */
        [[nodiscard]] std::unique_ptr<Matrix> create_substituted_matrix(SymbolTable& symbols, const Matrix& matrix) const;


        /**
         * Gets name of rulebook.
         */
        [[nodiscard]] const std::string& name() const noexcept { return this->human_readable_name; }

        /**
         * Sets name of rulebook.
         */
        void set_name(std::string the_name) noexcept { this->human_readable_name = std::move(the_name); }


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
         * True if rulebook has been applied to at least one matrix.
         */
         [[nodiscard]] const bool in_use() {
             return this->usages.load(std::memory_order_acquire) > 0;
         }

         /**
          * Finds pointer to the first rule in RHS rulebook that is not implied by this rulebook, or nullptr otherwise.
          */
          [[nodiscard]] const MomentRule *
          first_noncontained_rule(const MomentRulebook& rhs) const;

          /**
           * Compare rulebook
           */
           std::tuple<RulebookComparisonResult, const MomentRule *, const MomentRule *>
           compare_rulebooks(const MomentRulebook& rhs) const;

    private:
            /**
             * Regenerate ordered rule keys.
             */
            void remake_keys();
    };
}