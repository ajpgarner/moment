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
    };

    /**
     * Rule, matching symbol ID and replacing it with a polynomial.
     */
    class MomentSubstitutionRule {

    private:
        symbol_name_t lhs;
        Polynomial rhs;

    public:
        /** Create rule: symbol_id -> polynomial. */
        MomentSubstitutionRule(symbol_name_t lhs, Polynomial&& rhs)
            : lhs{lhs}, rhs{std::move(rhs)} { }

        /** Create rule from polynomial == 0. */
        MomentSubstitutionRule(const SymbolTable& table, Polynomial&& rule);

    public:
        /**
         * Match pattern.
         */
        [[nodiscard]] symbol_name_t LHS() const noexcept { return this->lhs; }

        /**
         * Replacement string.
         */
        [[nodiscard]] const Polynomial& RHS() const noexcept { return this->rhs; }

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
         */
        template<typename inserter_iter_t>
        inline void append_transformed(const Monomial& match, inserter_iter_t insert_iter) const {
            assert(match.id == this->lhs);
            if (match.conjugated) {
                std::transform(rhs.begin(), rhs.end(), insert_iter,
                               [&match](Monomial src) {
                                   src.conjugated = !src.conjugated;
                                   src.factor = match.factor * std::conj(src.factor);
                                   return src;
                               });
            } else {
                std::transform(rhs.begin(), rhs.end(), insert_iter,
                               [&match](Monomial src) {
                                   src.factor *= match.factor;
                                   return src;
                               });
            }
        }

        friend class MomentSubstitutionRulebook;
    };
}