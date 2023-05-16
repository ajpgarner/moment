/**
 * order_symbols_by_hash.h
 *
 * @copyright Copyright (c) 2023 Austrian Academy of Sciences
 * @author Andrew J. P. Garner
 */

#pragma once

#include "symbol_combo.h"
#include "symbol_table.h"

namespace Moment {

    /**
     * Order first by operator hash of forward sequence, then by conjugation sequence.
     * This is not quite the same as op-hash order; because it guarantees complex conjugate strings are adjacent.
     */
    struct CompareByOpHash {
    public:
        const SymbolTable& symbolTable;

        explicit CompareByOpHash(const SymbolTable& symbolTable)
                : symbolTable{symbolTable} { }

        [[nodiscard]] bool operator()(const SymbolExpression& lhs, const SymbolExpression& rhs) const noexcept;
    };

    /**
     * Factory, construct symbols from blah.
     */

    class ByHashSymbolComboFactory : public SymbolComboFactory {
    private:
        CompareByOpHash comparator;

    public:
        explicit ByHashSymbolComboFactory(const SymbolTable& symbols)
            : SymbolComboFactory{symbols}, comparator{symbols} { }

        SymbolCombo operator()(SymbolCombo::storage_t &&data) const override {
            return SymbolCombo(data, this->symbols, this->comparator);
        }

    };


}