/**
 * moment_substitution_rulebook.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once
#include "moment_rule.h"

#include "multithreading/multithreading.h"

#include <atomic>
#include <map>
#include <memory>
#include <tuple>
#include <vector>

namespace Moment {
    class SymbolicMatrix;
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
         * Associated operator context (mainly for error messages).
         */
        const Context& context;

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

        /**
         * True if RHS of every rule is a monomial.
         */
        bool monomial_rules = true;

        /**
         * True if rules do not break Hermiticity.
         */
        bool hermitian_rules = true;

        /**
         * True if extra rules can be added to account for factorization relationships
         */
        bool allow_safe_updates = true;

        /**
         * Counts how many matrices this rulebook has been applied to.
         */
        mutable std::atomic<size_t> usages = 0;

        /**
         * Disables 'usage' checks before adding rules.
         */
        bool in_expansion_mode = false;

    public:
        /**
         * Constructs a moment rulebook.
         * @param system The associated matrix system.
         * @param allow_safe_updates
         */
        explicit MomentRulebook(const MatrixSystem& system, bool allow_safe_updates = true);

        /**
         * No copy constructor.
         */
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
         * Number of pending 'raw' rules
         */
        [[nodiscard]] inline size_t raw_rule_size() const noexcept { return this->raw_rules.size(); }

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
         * Find rule, by LHS.
         */
        [[nodiscard]] auto find(const symbol_name_t symbol_id) const noexcept {
            return this->rules.find(symbol_id);
        }

        /**
         * Find first matching rule.
         * @returns Pair: iterator to matching rule, iterator to matching monomial element (or end, end).
         * If one output is not end, other is guaranteed to not be end.
         */
        [[nodiscard]] std::pair<rule_map_t::const_iterator, Polynomial::storage_t::const_iterator>
        match(const Polynomial& test) const noexcept;

        /**
         * Can only find match in l-values.
         */
        [[nodiscard]] std::pair<rule_map_t::const_iterator, Polynomial::storage_t::const_iterator>
        match(Polynomial&& test) const = delete;

        /**
         * Apply reduction to every element of matrix, and make a new matrix
         * @param symbols Write-access to symbol table.
         * @param matrix  The matrix to substitute.
         * @param mt_policy Whether or not to use multi-threading.
         * @return Newly created matrix, either of type MonomialSubstitutionMatrix or PolynomialSubstitutionMatrix.
         */
        [[nodiscard]] std::unique_ptr<SymbolicMatrix>
        create_substituted_matrix(SymbolTable& symbols, const SymbolicMatrix& matrix,
              Multithreading::MultiThreadPolicy mt_policy = Multithreading::MultiThreadPolicy::Optional) const;


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
          * Call to enable writing of rules, even if rulebook is in use.
          * Undefined behaviour if any new rules (or their completion) added would result in a /different/ result when
          * applied to an object that the rulebook was previous applied to.
          *
          * Useful for things like adding new factor behaviour, etc.
          * @return True, if expansion is allowed at all
          */
         inline bool enable_expansion() noexcept {
             if (this->allow_safe_updates) {
                 this->in_expansion_mode = true;
                 return true;
             }
             return false;
         }

         /**
          * Flag that 'safe' expansion mode is over.
          */
         inline void disable_expansion() noexcept {
             this->in_expansion_mode = false;
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