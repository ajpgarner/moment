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

    class MomentSubstitutionRule {

    private:
        const SymbolTable * table;
        symbol_name_t lhs;
        SymbolCombo rhs;

    public:
        /** Create rule from symbol_id, and polynomial. */
        MomentSubstitutionRule(const SymbolTable& table, symbol_name_t lhs, SymbolCombo&& rhs)
            : table{&table}, lhs{lhs}, rhs{std::move(rhs)} { }

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

    };
}