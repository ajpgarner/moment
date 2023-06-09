/**
 * moment_substitution_rule.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "polynomial.h"

#include <algorithm>

namespace Moment {
    class SymbolTable;

    namespace errors {
        class invalid_moment_rule : public std::invalid_argument {
        public:
            const symbol_name_t lhs_id;

        public:
            invalid_moment_rule(const symbol_name_t sym_id, const std::string &what)
                    : std::invalid_argument(what), lhs_id{sym_id} {}
        };

        class nonorientable_rule : public invalid_moment_rule {
        public:
            explicit nonorientable_rule(const symbol_name_t sym_id,  const std::string &what)
                : invalid_moment_rule{sym_id, what} { }
        };
    };

    /**
     * Rule, matching symbol ID and replacing it with a polynomial.
     */
    class MomentSubstitutionRule {
    public:
        enum class PolynomialDifficulty {
            /** Rule has not been tested for difficulty (or even validity) */
            Unknown,
            /** Rule is 0 == 0 */
            Trivial,
            /** Rule is 1 = k, where k is a scalar not equal to 1. */
            Contradiction,
            /** Rule is straightforwardly orientable (leading term appears without its conjugate). */
            Simple,
            /** Rule contains leading term and its conjugate, but can be rearranged to be Simple. */
            NeedsReorienting,
            /** Rule contains leading term and conjugate in such a way that the rule only partially constraints term. */
            NonorientableRule
        };

    private:
        symbol_name_t lhs;
        Polynomial rhs;

    public:
        /** Create rule directly: symbol_id -> polynomial. */
        MomentSubstitutionRule(symbol_name_t lhs, Polynomial&& rhs)
            : lhs{lhs}, rhs{std::move(rhs)} { }

    private:
        MomentSubstitutionRule(const PolynomialFactory& factory, Polynomial&& rule, PolynomialDifficulty difficulty);


    public:
        /** Create rule from polynomial == 0. */
        MomentSubstitutionRule(const PolynomialFactory& factory, Polynomial&& rule);

    public:
        /**
         * Check if LHS is (anti)-Hermitian, and if so, split the rule in two, returning the second polynomial.
         */
        [[nodiscard]] std::optional<Polynomial> impose_hermicity_of_LHS(const PolynomialFactory& factory);


        /**
         * Match pattern.
         */
        [[nodiscard]] symbol_name_t LHS() const noexcept { return this->lhs; }

        /**
         * Replacement string.
         */
        [[nodiscard]] const Polynomial& RHS() const noexcept { return this->rhs; }

        /**
         * Copy of entire rule as a polynomial (-LHS + RHS = 0)
         */
        [[nodiscard]] Polynomial as_polynomial(const PolynomialFactory& factory) const;

        /**
         * True if rule has non-trivial action on supplied combo.
         */
        [[nodiscard]] bool matches(const Polynomial& combo) const noexcept;


        /**
         * Checks if rule matches zero, one or two times (factoring complex conjugation), and return hint pointer.
         */
        [[nodiscard]] std::pair<size_t, Polynomial::storage_t::const_iterator>
        match_info(const Polynomial& combo) const noexcept;


        /**
         * Act with rule on combo to make new combo.
         */
        [[nodiscard]] Polynomial reduce(const PolynomialFactory& factory, const Polynomial& rhs) const;


        /**
         * Act with rule on symbol expression to make combo.
         */
        [[nodiscard]] Polynomial reduce(const PolynomialFactory& factory, const Monomial& rhs) const;

        /**
         * Try to act with rule on symbol expression to make monomial
         */
        [[nodiscard]] Monomial reduce_monomial(const SymbolTable& table,
                                               const Monomial& rhs) const;

        /**
         * Act with rule on combo to make new combo, using binding hint.
         * Will crash if hint is incorrect!
         *
         * @param factory The object for constructing new symbol combos (encodes sorting order).
         * @param rhs The combo to reduce.
         * @param hint Iterator to first match
         * @param twice True if matches symbol and its CC
         * @return Reduced combo.
         */
        [[nodiscard]] Polynomial reduce_with_hint(const PolynomialFactory& factory,
                                                  const Polynomial& rhs,
                                                  Polynomial::storage_t::const_iterator hint,
                                                  bool twice = false) const;

        /**
         * Is rule effectively empty?
         */
        [[nodiscard]] inline bool is_trivial() const noexcept { return this->lhs == 0; }

        /**
         * Write out the RHS of the rule, up to conjugation and factors.
         *
         * @tparam inserter_iter_t
         * @param match The monomial that matches the LHS of this rule.
         * @param insert_iter Iterator to where in, e.g., a Polynomial, to insert the pattern.
         */
        template<typename inserter_iter_t>
        inline void append_transformed(const Monomial& match, inserter_iter_t insert_iter) const {
            assert(match.id == this->lhs);
            if (match.conjugated) {
                std::transform(this->rhs.begin(), this->rhs.end(), insert_iter,
                               [&match](Monomial src) {
                                   src.conjugated = !src.conjugated;
                                   src.factor = match.factor * std::conj(src.factor);
                                   return src;
                               });
            } else {
                std::transform(this->rhs.begin(), this->rhs.end(), insert_iter,
                               [&match](Monomial src) {
                                   src.factor *= match.factor;
                                   return src;
                               });
            }
        }

        /**
         * Judge the difficulty of a Polynomial to orient into a rule.
         * @param poly The polynomial to test
         * @param tolerance The tolerance factor for double equality.
         */
        [[nodiscard]] static PolynomialDifficulty get_difficulty(const Polynomial& poly,
                                                                 double tolerance = 1.0) noexcept;



    private:
        /**
         * Processes RHS into proper rule.
         * Ideally would be in constructor, but can't guarantee order of input evaluation.
         */
        void set_up_rule(const PolynomialFactory& factory, PolynomialDifficulty difficulty);

        /**
         * Re-orient a Polynomial of the form a X + b X* + P == 0, where P contains neither X or X* and |a| != |b|.
         * @return Equivalent polynomial of the form X + Q == 0, where Q is a polynomial containing neither X or X*.
         */
        [[nodiscard]] static Polynomial reorient_polynomial(const PolynomialFactory& factory, Polynomial input_rule);

    public:
        friend class MomentSubstitutionRulebook;
    };
}