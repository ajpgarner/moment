/**
 * moment_substitution_rule.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "symbol_combo.h"

namespace Moment {
    class SymbolTable;

    namespace errors {
        class invalid_moment_rule : public std::invalid_argument {
        public:
            const symbol_name_t lhs_id;

        public:
            invalid_moment_rule(const symbol_name_t sym_id, const std::string& what)
                : std::invalid_argument(what), lhs_id{sym_id} { }
        };
    };


    class MomentSubstitutionRule {

    private:
        const SymbolTable * table;
        symbol_name_t lhs;
        SymbolCombo rhs;

    public:
        /** Create rule: symbol_id -> polynomial. */
        MomentSubstitutionRule(const SymbolTable& table, symbol_name_t lhs, SymbolCombo&& rhs)
            : table{&table}, lhs{lhs}, rhs{std::move(rhs)} { }

        /** Create rule from polynomial == 0. */
        MomentSubstitutionRule(const SymbolTable& table, SymbolCombo&& rule);

    public:
        /**
         * Match pattern.
         */
        [[nodiscard]] symbol_name_t LHS() const noexcept { return this->lhs; }

        /**
         * Replacement string.
         */
        [[nodiscard]] const SymbolCombo& RHS() const noexcept { return this->rhs; }

        /**
         * True if rule has non-trivial action on supplied combo.
         */
        [[nodiscard]] bool matches(const SymbolCombo& combo) const noexcept;

        /**
         * Act with rule on combo to make new combo
         */
        [[nodiscard]] SymbolCombo reduce(const SymbolCombo& rhs) const;

        /**
         * Is rule effectively empty?
         */
        [[nodiscard]] inline bool is_trivial() const noexcept { return this->lhs == 0; }

    };
}